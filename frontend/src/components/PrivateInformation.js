import React, { useState } from 'react';

const PrivateInformation = ({ getToken }) => {
  const [showPrivateInfo, setShowPrivateInfo] = useState(false);
  const [confirmationPending, setConfirmationPending] = useState(false);
  const [selectedDevice, setSelectedDevice] = useState('');
  const [devices, setDevices] = useState([]);
  const [confirmationId, setConfirmationId] = useState(null);
  
  React.useEffect(() => {
    // Fetch user's devices when component mounts
    fetchDevices();
    
    // Set up WebSocket listener for confirmation
    const setupWebSocket = async () => {
      try {
        const wsUrl = process.env.REACT_APP_WS_URL || 'ws://localhost:3000';
        const ws = new WebSocket(wsUrl);
        
        ws.onopen = async () => {
          console.log('WebSocket connected for private info');
          
          // Authenticate with the WebSocket server
          try {
            const token = await getToken();
            const response = await fetch('/api/user-id', {
              headers: {
                'Authorization': `Bearer ${token}`,
              },
            });
            
            if (response.ok) {
              const userData = await response.json();
              ws.send(JSON.stringify({ 
                type: 'auth', 
                userId: userData.userId 
              }));
            }
          } catch (error) {
            console.error('Error authenticating WebSocket:', error);
          }
        };
        
        ws.onmessage = (event) => {
          try {
            const message = JSON.parse(event.data);
            console.log('WebSocket message received:', message);
            
            if (message.type === 'confirmation_success' && message.confirmationId === confirmationId) {
              console.log('Confirmation successful! Showing private information...');
              // Show private information when confirmation succeeds
              setShowPrivateInfo(true);
              setConfirmationPending(false);
            } else if (message.type === 'confirmation_failed' && message.confirmationId === confirmationId) {
              console.log('Confirmation failed!');
              alert('Confirmation failed: ' + (message.error || 'Unknown error'));
              setConfirmationPending(false);
            }
          } catch (error) {
            console.error('Error parsing WebSocket message:', error);
          }
        };
        
        ws.onerror = (error) => {
          console.error('WebSocket error:', error);
        };
        
        return ws;
      } catch (error) {
        console.error('Error setting up WebSocket:', error);
        return null;
      }
    };
    
    setupWebSocket().then(ws => {
      if (ws) {
        return () => ws.close();
      }
    });
  }, [confirmationId, getToken]);
  
  const fetchDevices = async () => {
    try {
      const token = await getToken();
      const response = await fetch('/api/devices', {
        headers: {
          'Authorization': `Bearer ${token}`,
        },
      });
      
      if (response.ok) {
        const data = await response.json();
        setDevices(data.devices);
        if (data.devices.length > 0 && !selectedDevice) {
          setSelectedDevice(data.devices[0].deviceId);
        }
      }
    } catch (error) {
      console.error('Error fetching devices:', error);
    }
  };
  
  const requestAccess = async () => {
    if (!selectedDevice) {
      alert('Please select a device or register a new one in the Devices tab');
      return;
    }
    
    setConfirmationPending(true);
    try {
      const token = await getToken();
      const response = await fetch('/api/request-confirm', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
          'Authorization': `Bearer ${token}`,
        },
        body: JSON.stringify({
          deviceId: selectedDevice,
          action: 'Access Private Information'
        }),
      });
      
      if (response.ok) {
        const data = await response.json();
        const newConfirmationId = data.confirmationId;
        setConfirmationId(newConfirmationId);
        
        // Start polling for confirmation status
        startPollingForConfirmation(newConfirmationId);
      } else {
        const error = await response.json();
        alert(`Failed to request confirmation: ${error.error}`);
        setConfirmationPending(false);
      }
    } catch (error) {
      console.error('Error requesting confirmation:', error);
      alert('Error requesting confirmation. Please try again.');
      setConfirmationPending(false);
    }
  };

  const startPollingForConfirmation = async (confirmationId) => {
    const maxAttempts = 60; // Poll for 5 minutes (60 * 5 seconds)
    let attempts = 0;
    
    const pollInterval = setInterval(async () => {
      attempts++;
      
      if (attempts > maxAttempts) {
        clearInterval(pollInterval);
        setConfirmationPending(false);
        alert('Confirmation timeout. Please try again.');
        return;
      }
      
      try {
        const token = await getToken();
        const response = await fetch(`/api/confirmation/${confirmationId}`, {
          headers: {
            'Authorization': `Bearer ${token}`,
          },
        });
        
        if (response.ok) {
          const data = await response.json();
          console.log('Confirmation status:', data);
          
          if (data.status === 'confirmed') {
            clearInterval(pollInterval);
            setShowPrivateInfo(true);
            setConfirmationPending(false);
            console.log('✅ Confirmation successful! Showing private information.');
          } else if (data.status === 'failed' || data.status === 'expired') {
            clearInterval(pollInterval);
            setConfirmationPending(false);
            alert(`Confirmation ${data.status}. Please try again.`);
          }
        } else {
          console.error('Error checking confirmation status:', response.status);
        }
      } catch (error) {
        console.error('Error polling confirmation status:', error);
      }
    }, 5000); // Poll every 5 seconds
    
    // Clean up interval if component unmounts
    return () => clearInterval(pollInterval);
  };
  
  if (devices.length === 0) {
    return (
      <div className="empty-state">
        <h3>No devices available</h3>
        <p>Please register a device first in the "My Devices" tab to access private information.</p>
      </div>
    );
  }
  
  return (
    <div>
      <h3>🔒 Private Information</h3>
      
      {!showPrivateInfo ? (
        <div>
          <p style={{ color: '#666', marginBottom: '2rem' }}>
            This information is protected and requires physical confirmation to access.
            Your ESP32 device will light up and wait for you to press the button.
          </p>
          
          {!confirmationPending ? (
            <div>
              <div className="form-group">
                <label>Select Confirmation Device</label>
                <select
                  value={selectedDevice}
                  onChange={(e) => setSelectedDevice(e.target.value)}
                  style={{ padding: '12px', borderRadius: '8px', border: '2px solid #e9ecef', width: '100%' }}
                >
                  {devices.map((device) => (
                    <option key={device.deviceId} value={device.deviceId}>
                      {device.deviceName} ({device.deviceId})
                    </option>
                  ))}
                </select>
              </div>
              
              <button 
                className="btn btn-success" 
                onClick={requestAccess}
                style={{ fontSize: '1.1rem', padding: '15px 30px' }}
              >
                🔐 Request Access to Private Information
              </button>
            </div>
          ) : (
            <div style={{ padding: '1.5rem', background: '#fff3cd', borderRadius: '8px', border: '1px solid #ffeaa7' }}>
              <h4 style={{ margin: '0 0 1rem 0', color: '#856404' }}>
                ⏳ Waiting for Physical Confirmation
              </h4>
              <p style={{ margin: '0 0 1rem 0', color: '#856404' }}>
                Your ESP32 device should be lighting up. Press the button on your device to confirm access.
              </p>
              <p style={{ margin: '0 0 1rem 0', color: '#856404', fontSize: '0.9rem' }}>
                Confirmation ID: {confirmationId}
              </p>
              <button 
                className="btn" 
                onClick={() => {
                  setConfirmationPending(false);
                  setConfirmationId(null);
                }}
                style={{ marginTop: '0.5rem' }}
              >
                Cancel
              </button>
            </div>
          )}
        </div>
      ) : (
        <div style={{ padding: '2rem', background: '#d4edda', borderRadius: '8px', border: '1px solid #c3e6cb' }}>
          <h4 style={{ color: '#155724', marginBottom: '1rem' }}>✅ Access Granted</h4>
          
          <div style={{ background: 'white', padding: '1.5rem', borderRadius: '8px' }}>
            <h5>Sensitive Account Information</h5>
            <p><strong>Account Number:</strong> 1234-5678-9012-3456</p>
            <p><strong>Account Balance:</strong> $10,245.67</p>
            <p><strong>Last Transaction:</strong> $156.78 at Grocery Store on Sep 17, 2025</p>
            <p><strong>Social Security:</strong> XXX-XX-1234</p>
            <p><strong>Home Address:</strong> 123 Security Lane, Private City, CA 90210</p>
          </div>
          
          <button 
            className="btn" 
            onClick={() => setShowPrivateInfo(false)}
            style={{ marginTop: '1rem' }}
          >
            Hide Private Information
          </button>
        </div>
      )}
    </div>
  );
};

export default PrivateInformation;

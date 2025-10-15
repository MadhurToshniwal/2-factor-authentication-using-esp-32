import React, { useState, useEffect } from 'react';

const ConfirmationDemo = ({ getToken }) => {
  const [devices, setDevices] = useState([]);
  const [selectedDevice, setSelectedDevice] = useState('');
  const [customAction, setCustomAction] = useState('Test Secure Action');
  const [loading, setLoading] = useState(false);
  const [pendingConfirmation, setPendingConfirmation] = useState(null);

  useEffect(() => {
    fetchDevices();
  }, []);

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

  const requestConfirmation = async () => {
    if (!selectedDevice) {
      alert('Please select a device');
      return;
    }

    setLoading(true);
    setPendingConfirmation(null);

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
          action: customAction
        }),
      });

      if (response.ok) {
        const data = await response.json();
        setPendingConfirmation({
          confirmationId: data.confirmationId,
          action: customAction,
          deviceId: selectedDevice,
          timestamp: Date.now()
        });
      } else {
        const error = await response.json();
        alert(`Failed to request confirmation: ${error.error}`);
      }
    } catch (error) {
      console.error('Error requesting confirmation:', error);
      alert('Error requesting confirmation. Please try again.');
    } finally {
      setLoading(false);
    }
  };

  const checkConfirmationStatus = async (confirmationId) => {
    try {
      const token = await getToken();
      const response = await fetch(`/api/confirmation/${confirmationId}`, {
        headers: {
          'Authorization': `Bearer ${token}`,
        },
      });

      if (response.ok) {
        const data = await response.json();
        return data.status;
      }
    } catch (error) {
      console.error('Error checking confirmation status:', error);
    }
    return null;
  };

  const cancelConfirmation = () => {
    setPendingConfirmation(null);
  };

  if (devices.length === 0) {
    return (
      <div className="empty-state">
        <h3>No devices available</h3>
        <p>Please register a device first to test confirmations.</p>
      </div>
    );
  }

  return (
    <div>
      <h3>🧪 Test 2FA Confirmation</h3>
      <p style={{ color: '#666', marginBottom: '2rem' }}>
        Test your physical confirmation device by requesting a secure action confirmation.
        Your device will light up and wait for you to press the button.
      </p>

      <div style={{ display: 'grid', gap: '1rem', marginBottom: '2rem' }}>
        <div className="form-group">
          <label>Select Device</label>
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

        <div className="form-group">
          <label>Action to Confirm</label>
          <input
            type="text"
            value={customAction}
            onChange={(e) => setCustomAction(e.target.value)}
            placeholder="Enter what action you want to confirm"
          />
        </div>
      </div>

      {!pendingConfirmation ? (
        <button 
          className="btn btn-success" 
          onClick={requestConfirmation}
          disabled={loading}
          style={{ fontSize: '1.1rem', padding: '15px 30px' }}
        >
          {loading && <div className="loading"></div>}
          🔒 Request Confirmation
        </button>
      ) : (
        <div style={{ padding: '1.5rem', background: '#fff3cd', borderRadius: '8px', border: '1px solid #ffeaa7' }}>
          <h4 style={{ margin: '0 0 1rem 0', color: '#856404' }}>
            ⏳ Waiting for Device Confirmation
          </h4>
          <p style={{ margin: '0 0 1rem 0', color: '#856404' }}>
            <strong>Action:</strong> {pendingConfirmation.action}<br/>
            <strong>Device:</strong> {devices.find(d => d.deviceId === pendingConfirmation.deviceId)?.deviceName}<br/>
            <strong>Sent:</strong> {new Date(pendingConfirmation.timestamp).toLocaleString()}
          </p>
          <p style={{ margin: '0 0 1rem 0', color: '#856404' }}>
            🔔 Your device should be lighting up. Press the button on your wearable to confirm this action.
          </p>
          <button 
            className="btn btn-secondary"
            onClick={cancelConfirmation}
          >
            Cancel
          </button>
        </div>
      )}

      <div style={{ marginTop: '2rem', padding: '1rem', background: '#e7f3ff', borderRadius: '8px' }}>
        <h4>💡 How it works:</h4>
        <ol style={{ paddingLeft: '1.5rem', color: '#666' }}>
          <li>Click "Request Confirmation" to send a challenge to your device</li>
          <li>Your ESP32 device receives the challenge via MQTT</li>
          <li>The LED on your device will light up</li>
          <li>Press the button on your device to confirm</li>
          <li>The device signs the challenge and sends it back</li>
          <li>You'll get a real-time notification when confirmed!</li>
        </ol>
      </div>

      <div style={{ marginTop: '1rem', padding: '1rem', background: '#f8f9fa', borderRadius: '8px' }}>
        <h4>🔧 Troubleshooting:</h4>
        <ul style={{ paddingLeft: '1.5rem', color: '#666' }}>
          <li>Make sure your ESP32 device is powered on and connected to WiFi</li>
          <li>Check that the device is connected to the MQTT broker (see serial monitor)</li>
          <li>Ensure the device LED lights up when you request confirmation</li>
          <li>If no LED, check your device registration and MQTT connection</li>
        </ul>
      </div>
    </div>
  );
};

export default ConfirmationDemo;

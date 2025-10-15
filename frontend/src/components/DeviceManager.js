import React, { useState, useEffect } from 'react';

const DeviceManager = ({ getToken }) => {
  const [devices, setDevices] = useState([]);
  const [loading, setLoading] = useState(true);
  const [showAddForm, setShowAddForm] = useState(false);
  const [newDevice, setNewDevice] = useState({
    deviceId: '',
    deviceSecret: '',
    deviceName: ''
  });
  const [addingDevice, setAddingDevice] = useState(false);

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
      } else {
        console.error('Failed to fetch devices');
      }
    } catch (error) {
      console.error('Error fetching devices:', error);
    } finally {
      setLoading(false);
    }
  };

  const handleAddDevice = async (e) => {
    e.preventDefault();
    if (!newDevice.deviceId || !newDevice.deviceSecret) {
      alert('Please fill in all required fields');
      return;
    }

    setAddingDevice(true);
    try {
      const token = await getToken();
      const response = await fetch('/api/register-device', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
          'Authorization': `Bearer ${token}`,
        },
        body: JSON.stringify(newDevice),
      });

      if (response.ok) {
        const data = await response.json();
        alert(`Device "${data.deviceName}" registered successfully!`);
        setNewDevice({ deviceId: '', deviceSecret: '', deviceName: '' });
        setShowAddForm(false);
        fetchDevices(); // Refresh the list
      } else {
        const error = await response.json();
        alert(`Failed to register device: ${error.error}`);
      }
    } catch (error) {
      console.error('Error adding device:', error);
      alert('Error adding device. Please try again.');
    } finally {
      setAddingDevice(false);
    }
  };

  const handleRemoveDevice = async (deviceId) => {
    if (!window.confirm('Are you sure you want to remove this device?')) {
      return;
    }

    try {
      const token = await getToken();
      const response = await fetch(`/api/devices/${deviceId}`, {
        method: 'DELETE',
        headers: {
          'Authorization': `Bearer ${token}`,
        },
      });

      if (response.ok) {
        alert('Device removed successfully');
        fetchDevices(); // Refresh the list
      } else {
        const error = await response.json();
        alert(`Failed to remove device: ${error.error}`);
      }
    } catch (error) {
      console.error('Error removing device:', error);
      alert('Error removing device. Please try again.');
    }
  };

  if (loading) {
    return (
      <div>
        <div className="loading"></div>
        Loading devices...
      </div>
    );
  }

  return (
    <div>
      <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', marginBottom: '2rem' }}>
        <h3>My Registered Devices</h3>
        <button 
          className="btn" 
          onClick={() => setShowAddForm(!showAddForm)}
        >
          {showAddForm ? 'Cancel' : '+ Add Device'}
        </button>
      </div>

      {showAddForm && (
        <form onSubmit={handleAddDevice} style={{ marginBottom: '2rem', padding: '1.5rem', background: '#f8f9fa', borderRadius: '8px' }}>
          <h4>Register New Device</h4>
          <p style={{ color: '#666', fontSize: '0.9rem', marginBottom: '1rem' }}>
            Power on your ESP32 device and copy the Device ID and Secret from the serial monitor.
          </p>
          
          <div className="form-group">
            <label>Device ID *</label>
            <input
              type="text"
              value={newDevice.deviceId}
              onChange={(e) => setNewDevice({ ...newDevice, deviceId: e.target.value })}
              placeholder="e.g., AA:BB:CC:DD:EE:FF"
              required
            />
          </div>

          <div className="form-group">
            <label>Device Secret (Hex) *</label>
            <textarea
              value={newDevice.deviceSecret}
              onChange={(e) => setNewDevice({ ...newDevice, deviceSecret: e.target.value })}
              placeholder="64-character hex string from device serial output"
              required
              style={{ fontFamily: 'monospace', fontSize: '0.9rem' }}
            />
          </div>

          <div className="form-group">
            <label>Device Name (Optional)</label>
            <input
              type="text"
              value={newDevice.deviceName}
              onChange={(e) => setNewDevice({ ...newDevice, deviceName: e.target.value })}
              placeholder="e.g., My Wearable Band"
            />
          </div>

          <div>
            <button type="submit" className="btn" disabled={addingDevice}>
              {addingDevice && <div className="loading"></div>}
              Register Device
            </button>
            <button 
              type="button" 
              className="btn btn-secondary" 
              onClick={() => setShowAddForm(false)}
            >
              Cancel
            </button>
          </div>
        </form>
      )}

      {devices.length === 0 ? (
        <div className="empty-state">
          <h3>No devices registered</h3>
          <p>Register your ESP32-based 2FA device to get started.</p>
          <div style={{ marginTop: '1rem', padding: '1rem', background: '#e7f3ff', borderRadius: '8px', textAlign: 'left' }}>
            <h4>📋 Setup Instructions:</h4>
            <ol style={{ paddingLeft: '1.5rem', color: '#666' }}>
              <li>Flash the ESP32 firmware to your device</li>
              <li>Power on the device and connect to serial monitor</li>
              <li>Copy the Device ID and Secret from the serial output</li>
              <li>Click "Add Device" and paste the information</li>
              <li>Your device will be ready for 2FA confirmations!</li>
            </ol>
          </div>
        </div>
      ) : (
        <div className="device-list">
          {devices.map((device) => (
            <div key={device.deviceId} className="device-item">
              <div className="device-info">
                <h4>{device.deviceName}</h4>
                <p><strong>Device ID:</strong> {device.deviceId}</p>
                <p><strong>Registered:</strong> {new Date(device.registeredAt).toLocaleString()}</p>
                <p><strong>Last Seen:</strong> {new Date(device.lastSeen).toLocaleString()}</p>
              </div>
              <div className="device-actions">
                <button 
                  className="btn btn-danger"
                  onClick={() => handleRemoveDevice(device.deviceId)}
                >
                  Remove
                </button>
              </div>
            </div>
          ))}
        </div>
      )}
    </div>
  );
};

export default DeviceManager;

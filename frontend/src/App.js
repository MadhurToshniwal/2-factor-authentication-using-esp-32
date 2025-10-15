import React, { useState } from 'react';
import { useUser, useAuth, SignInButton, SignOutButton } from '@clerk/clerk-react';
import DeviceManager from './components/DeviceManager';
import ConfirmationDemo from './components/ConfirmationDemo';
import ConfirmationHistory from './components/ConfirmationHistory';
import PrivateInformation from './components/PrivateInformation';
import Notification from './components/Notification';
import useWebSocket from './hooks/useWebSocket';
import './index.css';

function App() {
  const { isSignedIn, user } = useUser();
  const { getToken } = useAuth();
  const [activeTab, setActiveTab] = useState('demo');
  const [notification, setNotification] = useState(null);

  // WebSocket for real-time updates
  useWebSocket(user?.id, (message) => {
    switch (message.type) {
      case 'confirmation_success':
        setNotification({
          type: 'success',
          message: `Action "${message.action}" confirmed successfully!`
        });
        break;
      case 'confirmation_failed':
        setNotification({
          type: 'error',
          message: `Confirmation failed: ${message.error}`
        });
        break;
      case 'confirmation_expired':
        setNotification({
          type: 'error',
          message: `Confirmation for "${message.action}" expired`
        });
        break;
      default:
        break;
    }
  });

  const closeNotification = () => {
    setNotification(null);
  };

  if (!isSignedIn) {
    return (
      <div className="App">
        <div className="auth-section">
          <div className="auth-card">
            <h2>🔐 Secure 2FA Band</h2>
            <p>
              Welcome to the Secure 2FA Confirmation Band system. This innovative 
              wearable device provides physical confirmation for your digital actions.
            </p>
            <p>
              Sign in with your Clerk account to get started with device registration 
              and secure confirmations.
            </p>
            <SignInButton mode="modal">
              <button className="btn">
                Sign In / Sign Up
              </button>
            </SignInButton>
          </div>
        </div>
        {notification && (
          <Notification
            type={notification.type}
            message={notification.message}
            onClose={closeNotification}
          />
        )}
      </div>
    );
  }

  return (
    <div className="App">
      <header className="header">
        <h1>🔐 Secure 2FA Band</h1>
        <div className="user-info">
          <span>Welcome, {user?.firstName || user?.emailAddresses[0]?.emailAddress}</span>
          <SignOutButton>
            <button className="btn btn-secondary">Sign Out</button>
          </SignOutButton>
        </div>
      </header>

      <main className="main-content">
        <div className="welcome-card">
          <h2>Physical 2FA Authentication</h2>
          <p>
            Your secure wearable device provides an additional layer of security by requiring 
            physical confirmation for sensitive actions. Register your ESP32-based device below 
            and test the confirmation system.
          </p>
        </div>

        <div className="device-card">
          <div className="tabs">
            <button 
              className={`tab ${activeTab === 'demo' ? 'active' : ''}`}
              onClick={() => setActiveTab('demo')}
            >
              Demo & Test
            </button>
            <button 
              className={`tab ${activeTab === 'private' ? 'active' : ''}`}
              onClick={() => setActiveTab('private')}
            >
              Private Information
            </button>
            <button 
              className={`tab ${activeTab === 'devices' ? 'active' : ''}`}
              onClick={() => setActiveTab('devices')}
            >
              My Devices
            </button>
            <button 
              className={`tab ${activeTab === 'history' ? 'active' : ''}`}
              onClick={() => setActiveTab('history')}
            >
              History
            </button>
          </div>

          {activeTab === 'demo' && <ConfirmationDemo getToken={getToken} />}
          {activeTab === 'private' && <PrivateInformation getToken={getToken} />}
          {activeTab === 'devices' && <DeviceManager getToken={getToken} />}
          {activeTab === 'history' && <ConfirmationHistory getToken={getToken} />}
        </div>
      </main>

      {notification && (
        <Notification
          type={notification.type}
          message={notification.message}
          onClose={closeNotification}
        />
      )}
    </div>
  );
}

export default App;

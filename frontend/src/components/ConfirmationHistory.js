import React, { useState, useEffect } from 'react';

const ConfirmationHistory = ({ getToken }) => {
  const [confirmations, setConfirmations] = useState([]);
  const [loading, setLoading] = useState(true);

  useEffect(() => {
    fetchConfirmations();
  }, []);

  const fetchConfirmations = async () => {
    try {
      const token = await getToken();
      const response = await fetch('/api/confirmations', {
        headers: {
          'Authorization': `Bearer ${token}`,
        },
      });

      if (response.ok) {
        const data = await response.json();
        setConfirmations(data.confirmations);
      } else {
        console.error('Failed to fetch confirmations');
      }
    } catch (error) {
      console.error('Error fetching confirmations:', error);
    } finally {
      setLoading(false);
    }
  };

  const getStatusColor = (status) => {
    switch (status) {
      case 'confirmed': return 'status-confirmed';
      case 'pending': return 'status-pending';
      case 'failed': return 'status-failed';
      case 'expired': return 'status-expired';
      default: return '';
    }
  };

  const formatTimestamp = (timestamp) => {
    return new Date(timestamp).toLocaleString();
  };

  const getActionEmoji = (action) => {
    if (action.toLowerCase().includes('login')) return '🔑';
    if (action.toLowerCase().includes('payment')) return '💳';
    if (action.toLowerCase().includes('delete')) return '🗑️';
    if (action.toLowerCase().includes('transfer')) return '💸';
    if (action.toLowerCase().includes('test')) return '🧪';
    return '🔒';
  };

  if (loading) {
    return (
      <div>
        <div className="loading"></div>
        Loading confirmation history...
      </div>
    );
  }

  return (
    <div>
      <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', marginBottom: '2rem' }}>
        <h3>📊 Confirmation History</h3>
        <button 
          className="btn btn-secondary" 
          onClick={fetchConfirmations}
        >
          🔄 Refresh
        </button>
      </div>

      {confirmations.length === 0 ? (
        <div className="empty-state">
          <h3>No confirmations yet</h3>
          <p>Your confirmation history will appear here once you start using your 2FA device.</p>
        </div>
      ) : (
        <div className="confirmation-history">
          {confirmations.map((confirmation) => (
            <div key={confirmation.confirmationId} className="confirmation-item">
              <div className="confirmation-details">
                <h4>
                  {getActionEmoji(confirmation.action)} {confirmation.action}
                </h4>
                <p>
                  <strong>Device:</strong> {confirmation.deviceId}<br/>
                  <strong>Requested:</strong> {formatTimestamp(confirmation.timestamp)}<br/>
                  <strong>ID:</strong> {confirmation.confirmationId}
                </p>
              </div>
              <div className="confirmation-actions">
                <span className={`status-indicator ${getStatusColor(confirmation.status)}`}>
                  {confirmation.status}
                </span>
              </div>
            </div>
          ))}
        </div>
      )}

      {confirmations.length > 0 && (
        <div style={{ marginTop: '2rem', padding: '1rem', background: '#f8f9fa', borderRadius: '8px' }}>
          <h4>📈 Summary</h4>
          <div style={{ display: 'grid', gridTemplateColumns: 'repeat(auto-fit, minmax(120px, 1fr))', gap: '1rem', marginTop: '1rem' }}>
            <div style={{ textAlign: 'center' }}>
              <div style={{ fontSize: '1.5rem', fontWeight: 'bold', color: '#28a745' }}>
                {confirmations.filter(c => c.status === 'confirmed').length}
              </div>
              <div style={{ fontSize: '0.9rem', color: '#666' }}>Confirmed</div>
            </div>
            <div style={{ textAlign: 'center' }}>
              <div style={{ fontSize: '1.5rem', fontWeight: 'bold', color: '#ffc107' }}>
                {confirmations.filter(c => c.status === 'pending').length}
              </div>
              <div style={{ fontSize: '0.9rem', color: '#666' }}>Pending</div>
            </div>
            <div style={{ textAlign: 'center' }}>
              <div style={{ fontSize: '1.5rem', fontWeight: 'bold', color: '#dc3545' }}>
                {confirmations.filter(c => c.status === 'failed').length}
              </div>
              <div style={{ fontSize: '0.9rem', color: '#666' }}>Failed</div>
            </div>
            <div style={{ textAlign: 'center' }}>
              <div style={{ fontSize: '1.5rem', fontWeight: 'bold', color: '#6c757d' }}>
                {confirmations.filter(c => c.status === 'expired').length}
              </div>
              <div style={{ fontSize: '0.9rem', color: '#666' }}>Expired</div>
            </div>
          </div>
        </div>
      )}
    </div>
  );
};

export default ConfirmationHistory;

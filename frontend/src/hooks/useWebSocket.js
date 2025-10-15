import { useEffect, useRef } from 'react';

const useWebSocket = (userId, onMessage) => {
  const ws = useRef(null);

  useEffect(() => {
    if (!userId) return;

    const wsUrl = process.env.REACT_APP_WS_URL || 'ws://localhost:3000';
    ws.current = new WebSocket(wsUrl);

    ws.current.onopen = () => {
      console.log('WebSocket connected');
      // Authenticate with userId
      ws.current.send(JSON.stringify({ type: 'auth', userId }));
    };

    ws.current.onmessage = (event) => {
      try {
        const message = JSON.parse(event.data);
        if (message.type === 'auth_success') {
          console.log('WebSocket authenticated');
        } else {
          onMessage(message);
        }
      } catch (error) {
        console.error('Error parsing WebSocket message:', error);
      }
    };

    ws.current.onclose = () => {
      console.log('WebSocket disconnected');
    };

    ws.current.onerror = (error) => {
      console.error('WebSocket error:', error);
    };

    return () => {
      if (ws.current) {
        ws.current.close();
      }
    };
  }, [userId, onMessage]);

  return ws.current;
};

export default useWebSocket;

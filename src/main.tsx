import React from 'react'
import ReactDOM from 'react-dom/client'
import App from './App.tsx'
import './index.css'

// Register service worker for PWA
// FORCE UNREGISTER service worker for debugging/updates
if ('serviceWorker' in navigator) {
  navigator.serviceWorker.getRegistrations().then(function (registrations) {
    for (let registration of registrations) {
      registration.unregister().then(bool => {
        console.log('SW Unregistered:', bool);
        // Force reload if we successfully unregistered and it's the first time
        // window.location.reload(); // Risky loop, let's just log it.
      });
    }
    // Also clear caches
    caches.keys().then(names => {
      for (let name of names) caches.delete(name);
      console.log('Caches cleared');
    });
  });
}

ReactDOM.createRoot(document.getElementById('root')!).render(
  <React.StrictMode>
    <App />
  </React.StrictMode>,
)

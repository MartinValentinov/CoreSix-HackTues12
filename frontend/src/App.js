import { useState } from "react";
import { BrowserRouter as Router, Routes, Route, Navigate } from "react-router-dom";
import Login from "./components/Login";
import Register from "./components/Register";
import Dashboard from "./components/Dashboard";

function App() {
  const [token, setToken] = useState(() => localStorage.getItem("token"));

  const handleAuthSuccess = (newToken) => {
    setToken(newToken);
  };

  const handleLogout = () => {
    localStorage.removeItem("token");
    setToken(null);
  };

  return (
    <Router>
      <div className="app">
        <header 
          className="app-header"
          style={{ marginLeft: "0px" }}
        >
          <div className="app-brand">
            <img
              className="app-logo"
              src="/logo192.png"
              alt="SupportCircle logo"
            />
            <span className="app-title">SupportCircle</span>
          </div>
        </header>
        <main className="app-main">
          <Routes>
            <Route path="/" element={<Navigate to="/login" />} />
            <Route
              path="/login"
              element={token ? <Navigate to="/dashboard" /> : <Login onAuthSuccess={handleAuthSuccess} />}
            />
            <Route
              path="/register"
              element={token ? <Navigate to="/dashboard" /> : <Register onAuthSuccess={handleAuthSuccess} />}
            />
            <Route
              path="/dashboard"
              element={token ? <Dashboard onLogout={handleLogout} /> : <Navigate to="/login" />}
            />
          </Routes>
        </main>
      </div>
    </Router>
  );
}

export default App;
import React, { useState } from "react";
import api from "../api/api";
import { useNavigate, Link } from "react-router-dom";

export default function Login({ onAuthSuccess }) {
  const [username, setUsername] = useState("");
  const [password, setPassword] = useState("");
  const navigate = useNavigate();

  const handleLogin = async (e) => {
    e.preventDefault();
    try {
      const res = await api.post("/auth/login", { username, password });
      localStorage.setItem("token", res.data.token);
      if (onAuthSuccess) {
        onAuthSuccess(res.data.token);
      }
      navigate("/dashboard");
    } catch (err) {
      console.error(err.response || err);
      alert(err.response?.data?.message || "Login failed");
    }
  };

  return (
    <div className="container">
      <div className="form-card">
        <h2>Login</h2>
        <form onSubmit={handleLogin}>
          <input
            type="text"
            placeholder="Username"
            value={username}
            onChange={(e) => setUsername(e.target.value)}
            required
          />
          <input
            type="password"
            placeholder="Password"
            value={password}
            onChange={(e) => setPassword(e.target.value)}
            required
          />
          <button type="submit">Login</button>
        </form>
        <p>
          Don't have an account? <Link to="/register">Register</Link>
        </p>
      </div>
    </div>
  );
}
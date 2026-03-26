import React, { useEffect, useState } from "react";
import api from "../api/api";
import { useNavigate } from "react-router-dom";

export default function Dashboard() {
  const [user, setUser] = useState(null);
  const navigate = useNavigate();

  useEffect(() => {
    const fetchUser = async () => {
      try {
        const res = await api.get("/auth/me"); // protected endpoint
        setUser(res.data);
      } catch (err) {
        console.error(err.response || err);
        navigate("/login");
      }
    };
    fetchUser();
  }, [navigate]);

  const handleLogout = () => {
    localStorage.removeItem("token");
    navigate("/login");
  };

  // Example posts for visual feed
  const feed = [
    { id: 1, title: "John's Journey", content: "Today I managed to walk 5 minutes without pain!" },
    { id: 2, title: "Anna's Progress", content: "Learning adaptive exercises has been amazing." },
    { id: 3, title: "Community Tip", content: "Stretching daily can reduce discomfort significantly." },
  ];

  return (
    <div className="container dashboard">
      <div className="profile">
        <div className="profile-avatar" />
        <div className="profile-name">{user?.username || "User"}</div>
      </div>
      <button onClick={handleLogout}>Logout</button>

      <div className="feed">
        {feed.map(post => (
          <div className="feed-card" key={post.id}>
            <h3>{post.title}</h3>
            <p>{post.content}</p>
          </div>
        ))}
      </div>
    </div>
  );
}
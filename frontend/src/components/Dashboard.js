import React, { useEffect, useState, useContext } from "react";
import api from "../api/api";
import { useNavigate } from "react-router-dom";
import { SidebarContext } from "../context/SidebarContext";

export default function Dashboard({ onLogout }) {
  const [user, setUser] = useState(null);
  const navigate = useNavigate();
  const { sidebarWidth, setSidebarWidth } = useContext(SidebarContext);

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

  const [activeTab, setActiveTab] = useState("home");
  const [darkMode, setDarkMode] = useState(false);
  const [isResizing, setIsResizing] = useState(false);
  const [selectedProfile, setSelectedProfile] = useState(null);

  useEffect(() => {
    document.body.classList.toggle("dark-theme", darkMode);
  }, [darkMode]);

  useEffect(() => {
    const handleMouseMove = (e) => {
      if (!isResizing) return;
      
      const newWidth = Math.max(150, Math.min(400, e.clientX));
      setSidebarWidth(newWidth);
    };

    const handleMouseUp = () => {
      setIsResizing(false);
    };

    if (isResizing) {
      document.addEventListener("mousemove", handleMouseMove);
      document.addEventListener("mouseup", handleMouseUp);
    }

    return () => {
      document.removeEventListener("mousemove", handleMouseMove);
      document.removeEventListener("mouseup", handleMouseUp);
    };
  }, [isResizing, setSidebarWidth]);

  const handleLogout = () => {
    if (onLogout) {
      onLogout();
      navigate("/login");
      return;
    }

    localStorage.removeItem("token");
    navigate("/login");
  };

  const [commentInputs, setCommentInputs] = useState({});

  const defaultAvatar = "https://upload.wikimedia.org/wikipedia/commons/8/89/Portrait_Placeholder.png";

  const initialFeed = [
    {
      id: 1,
      user: "John D.",
      avatar: defaultAvatar,
      time: "2h ago",
      content: "Today I managed to walk 5 minutes without pain! ",
      likes: 3,
      liked: false,
      comments: [{ user: "Anna K.", text: "Amazing progress!" }],
    },
    {
      id: 2,
      user: "Anna K.",
      time: "4h ago",
      content: "Adaptive exercises are changing my life.",
      likes: 5,
      liked: false,
      comments: [{ user: "Michael B.", text: "So inspiring!" }],
    },
    {
      id: 3,
      user: "Michael B.",
      time: "6h ago",
      content: "Small progress is still progress. Keep going everyone!",
      likes: 4,
      liked: false,
      comments: [],
    },
    {
      id: 4,
      user: "Sarah L.",
      time: "1 day ago",
      content: "Tried stretching before sleep — huge difference!",
      likes: 6,
      liked: false,
      comments: [],
    },
    {
      id: 5,
      user: "David R.",
      time: "1 day ago",
      content: "Grateful for this community 🙏",
      likes: 7,
      liked: false,
      comments: [{ user: "Anna K.", text: "Same here!" }],
    },
    {
      id: 6,
      user: "Emily T.",
      avatar: defaultAvatar,
      time: "2 days ago",
      content: "Managed to stay consistent for a week!",
      likes: 2,
      liked: false,
      comments: [],
    },
  ];

  const [feed, setFeed] = useState(initialFeed);
  const [newPostText, setNewPostText] = useState("");

  const handleAddPost = () => {
    const content = newPostText.trim();
    if (!content) return;

    const nextId = feed.length ? Math.max(...feed.map((p) => p.id)) + 1 : 1;
    const newPost = {
      id: nextId,
      user: user?.username || "You",
      time: "Just now",
      content,
      likes: 0,
      liked: false,
      comments: [],
    };

    setFeed((prev) => [newPost, ...prev]);
    setNewPostText("");
  };

  const handleLike = (postId) => {
    setFeed((prev) =>
      prev.map((post) =>
        post.id === postId
          ? {
              ...post,
              liked: !post.liked,
              likes: post.liked ? post.likes - 1 : post.likes + 1,
            }
          : post
      )
    );
  };

  const handleCommentInputChange = (postId, value) => {
    setCommentInputs((prev) => ({ ...prev, [postId]: value }));
  };

  const handleAddComment = (postId) => {
    const commentText = (commentInputs[postId] || "").trim();
    if (!commentText) return;

    const commentPayload = {
      user: user?.username || "Unknown",
      text: commentText,
    };

    setFeed((prev) =>
      prev.map((post) =>
        post.id === postId
          ? { ...post, comments: [...post.comments, commentPayload] }
          : post
      )
    );

    setCommentInputs((prev) => ({ ...prev, [postId]: "" }));
  };

  const profilePosts = selectedProfile
    ? feed.filter((post) => post.user === selectedProfile)
    : [];

  const profileCommentEntries = selectedProfile
    ? feed
        .flatMap((post) =>
          post.comments
            .filter((comment) => comment.user === selectedProfile)
            .map((comment) => ({
              postId: post.id,
              postUser: post.user,
              text: comment.text,
            }))
        )
    : [];

  const renderFeedCard = (post) => (
    <div className="feed-card" key={post.id}>
      <div className="feed-header">
        <img
          className="mini-avatar"
          src={post.avatar || defaultAvatar}
          alt={post.user}
        />
        <div>
          <strong
            className="profile-link"
            onClick={() => setSelectedProfile(post.user)}
          >
            {post.user}
          </strong>
          <div className="post-time">{post.time}</div>
        </div>
      </div>
      <p>{post.content}</p>
      <div className="feed-meta">
        <span>{post.likes} likes</span>
        <span>{post.comments.length} comments</span>
      </div>
      <div className="feed-actions">
        <button
          className="action-btn"
          onClick={() => handleLike(post.id)}
        >
          {post.liked ? "❤️ Liked" : "🤍 Like"}
        </button>
        <button
          className="action-btn"
          onClick={() => {
            const current = commentInputs[post.id] || "";
            const next = current.trim();
            if (next) {
              handleAddComment(post.id);
            }
          }}
        >
          💬 Add Comment
        </button>
      </div>
      <div className="comment-box">
        <input
          type="text"
          placeholder="Write a comment..."
          value={commentInputs[post.id] || ""}
          onChange={(e) =>
            handleCommentInputChange(post.id, e.target.value)
          }
          onKeyDown={(e) => {
            if (e.key === "Enter") {
              handleAddComment(post.id);
            }
          }}
        />
      </div>
      <div className="comment-list">
        {post.comments.map((comment, idx) => (
          <div className="comment-item" key={idx}>
            💬
            <strong
              className="comment-author"
              onClick={() => setSelectedProfile(comment.user)}
            >
              {comment.user}
            </strong>
            : {comment.text}
          </div>
        ))}
      </div>
    </div>
  );

  return (
  <div className="dashboard-layout">
    
    {/* Sidebar */}
    <aside className="sidebar" style={{ width: `${sidebarWidth}px` }}>
      <h2 className="logo">SupportCircle</h2>
      <nav>
        <button
          className={`nav-btn ${activeTab === "home" ? "active" : ""}`}
          onClick={() => setActiveTab("home")}
        >
          🏠 Home
        </button>
        <button
          className={`nav-btn ${activeTab === "community" ? "active" : ""}`}
          onClick={() => setActiveTab("community")}
        >
          💬 Community
        </button>
        <button
          className={`nav-btn ${activeTab === "settings" ? "active" : ""}`}
          onClick={() => setActiveTab("settings")}
        >
          ⚙️ Settings
        </button>
      </nav>
      <button className="logout-btn" onClick={handleLogout}>
        Logout
      </button>
      <div
        className="sidebar-resize-handle"
        onMouseDown={() => setIsResizing(true)}
      />
    </aside>

    {/* Main Content */}
    <div className="dashboard-content" style={{ marginLeft: `${sidebarWidth + 20}px` }}>
      
      {/* Top bar */}
      <div className="topbar">
        <div className="profile">
          <img
            className="profile-avatar"
            src={user?.avatar || defaultAvatar}
            alt="Profile"
          />
          <div className="profile-name">{user?.username || "User"}</div>
        </div>
      </div>

      {activeTab === "home" && (
        <>
          <div className="welcome-card">
            <h2>Welcome back 👋</h2>
            <p>You're doing great. Keep pushing forward!</p>
          </div>

          <div className="new-post-card">
            <textarea
              value={newPostText}
              onChange={(e) => setNewPostText(e.target.value)}
              placeholder="Share something with your community..."
            />
            <button className="post-btn" onClick={handleAddPost}>
              Post
            </button>
          </div>

          {selectedProfile && (
            <div className="profile-view">
              <button className="back-btn" onClick={() => setSelectedProfile(null)}>
                ← Back to community
              </button>
              <h3>{selectedProfile}'s profile</h3>
              <p>{profilePosts.length} post(s) authored</p>
              <p>{profileCommentEntries.length} comment(s) added</p>

              {profilePosts.length > 0 && (
                <div>
                  <h4>Author posts</h4>
                  {profilePosts.map((post) => renderFeedCard(post))}
                </div>
              )}

              {profileCommentEntries.length > 0 && (
                <div>
                  <h4>Comments on posts</h4>
                  {profileCommentEntries.map((entry, idx) => (
                    <div className="comment-item" key={idx}>
                      ✍️ post #{entry.postId} by {entry.postUser}: {entry.text}
                    </div>
                  ))}
                </div>
              )}
            </div>
          )}

          <div className="feed">
            {(selectedProfile ? profilePosts : feed).map((post) => renderFeedCard(post))}
          </div>
        </>
      )}

      {activeTab === "community" && (
        <>
          <div className="welcome-card">
            <h2>Community</h2>
            <p>Connect with others, share progress, and get support.</p>
          </div>

          {selectedProfile && (
            <div className="profile-view">
              <button className="back-btn" onClick={() => setSelectedProfile(null)}>
                ← Back to community
              </button>
              <h3>{selectedProfile}'s profile</h3>
              <p>{profilePosts.length} post(s) authored</p>
              <p>{profileCommentEntries.length} comment(s) added</p>

              {profilePosts.length > 0 && (
                <div>
                  <h4>Author posts</h4>
                  {profilePosts.map((post) => renderFeedCard(post))}
                </div>
              )}

              {profileCommentEntries.length > 0 && (
                <div>
                  <h4>Comments on posts</h4>
                  {profileCommentEntries.map((entry, idx) => (
                    <div className="comment-item" key={idx}>
                      ✍️ post #{entry.postId} by {entry.postUser}: {entry.text}
                    </div>
                  ))}
                </div>
              )}
            </div>
          )}

          <div className="feed">
            {(selectedProfile ? profilePosts : feed).map((post) => renderFeedCard(post))}
          </div>
        </>
      )}

      {activeTab === "settings" && (
        <div className="settings-container">
          <div className="settings-header">
            <h2>⚙️ Settings</h2>
            <p>Customize your experience</p>
          </div>

          {/* Appearance Settings */}
          <div className="settings-section">
            <div className="settings-section-title">
              <span>🎨 Appearance</span>
            </div>
            <div className="settings-group">
              <div className="settings-row">
                <div className="settings-label">
                  <strong>Dark Mode</strong>
                  <p>Switch to dark theme for comfortable night browsing</p>
                </div>
                <label className="toggle-switch">
                  <input
                    type="checkbox"
                    checked={darkMode}
                    onChange={(e) => setDarkMode(e.target.checked)}
                  />
                  <span className="toggle-slider"></span>
                </label>
              </div>
            </div>
          </div>

          {/* Notification Settings */}
          <div className="settings-section">
            <div className="settings-section-title">
              <span>🔔 Notifications</span>
            </div>
            <div className="settings-group">
              <div className="settings-row">
                <div className="settings-label">
                  <strong>Email Notifications</strong>
                  <p>Receive updates via email</p>
                </div>
                <label className="toggle-switch">
                  <input type="checkbox" defaultChecked={true} />
                  <span className="toggle-slider"></span>
                </label>
              </div>

              <div className="settings-row">
                <div className="settings-label">
                  <strong>Weekly Summary</strong>
                  <p>Get a summary of your activity every week</p>
                </div>
                <label className="toggle-switch">
                  <input type="checkbox" defaultChecked={false} />
                  <span className="toggle-slider"></span>
                </label>
              </div>
            </div>
          </div>


        </div>
      )}

    </div>
  </div>
);
}
import axios from "axios";

const API_URL = process.env.REACT_APP_API_URL || "/api";
export const API_ORIGIN = API_URL.startsWith("http")
  ? API_URL.replace(/\/api\/?$/, "")
  : window.location.origin;
const FORCE_NGROK_HEADER =
  process.env.REACT_APP_NGROK_SKIP_WARNING === "true" ||
  API_ORIGIN.includes("ngrok-free.dev");

const api = axios.create({
  baseURL: API_URL,
});

api.interceptors.request.use((config) => {
  const token = localStorage.getItem("token");
  if (token) {
    config.headers.Authorization = `Bearer ${token}`;
  }

  if (FORCE_NGROK_HEADER) {
    config.headers["ngrok-skip-browser-warning"] = "true";
  }

  return config;
});

api.interceptors.response.use(
  (response) => response,
  (error) => {
    if (error?.response?.status === 401) {
      localStorage.removeItem("token");
      window.dispatchEvent(new Event("auth:unauthorized"));
    }

    return Promise.reject(error);
  }
);

export default api;
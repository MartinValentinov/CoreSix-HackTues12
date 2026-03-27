import axios from "axios";

const API_URL = process.env.REACT_APP_API_URL || "/api";
export const API_ORIGIN = API_URL.startsWith("http")
  ? API_URL.replace(/\/api\/?$/, "")
  : window.location.origin;

const api = axios.create({
  baseURL: API_URL,
});

api.interceptors.request.use((config) => {
  const token = localStorage.getItem("token");
  if (token) {
    config.headers.Authorization = `Bearer ${token}`;
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
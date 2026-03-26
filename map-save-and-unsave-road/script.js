let map;
let routeLayer;

function initMap() {
  map = L.map('map').setView([42.6977, 23.3219], 15);

  L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
    attribution: '&copy; OpenStreetMap contributors'
  }).addTo(map);

  document.getElementById("safeRouteBtn").addEventListener("click", () => {
    showRoute("green", true);
  });

  document.getElementById("unsafeRouteBtn").addEventListener("click", () => {
    showRoute("red", false);
  });
}

async function geocode(place) {
  const url = `https://nominatim.openstreetmap.org/search?format=json&q=${encodeURIComponent(place)}`;
  const response = await fetch(url);
  const data = await response.json();
  return [parseFloat(data[0].lat), parseFloat(data[0].lon)];
}

async function getRoute(start, end) {
  const url = `https://router.project-osrm.org/route/v1/foot/${start[1]},${start[0]};${end[1]},${end[0]}?overview=full&geometries=geojson&alternatives=true`;
  const response = await fetch(url);
  return await response.json();
}

async function showRoute(color, safe) {
  if (routeLayer) {
    map.removeLayer(routeLayer);
  }

  const start = await geocode("Sofia University, Sofia");
  const end = await geocode("National Palace of Culture, Sofia");

  const data = await getRoute(start, end);

  let route = data.routes[0];

  if (!safe && data.routes.length > 1) {
    route = data.routes[1];
  }

  const coords = route.geometry.coordinates.map(c => [c[1], c[0]]);

  routeLayer = L.polyline(coords, {
    color: color,
    weight: 5
  }).addTo(map);

  map.fitBounds(routeLayer.getBounds());
}

window.onload = initMap;
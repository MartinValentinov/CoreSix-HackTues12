namespace Core.DTOs
{
    public class DistanceDto
    {
        public string? DeviceId { get; set; }  // nullable — ESP32 doesn't send this
        public float Distance { get; set; }
        public string? Alert { get; set; }
    }
}
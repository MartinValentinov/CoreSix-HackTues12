namespace Core.DTOs 
{
    public class RegisterResponseDto
    {
        public string? Token { get; set; }
        public string Username { get; set; } = null!;
        public string Email { get; set; } = null!;
    }
}
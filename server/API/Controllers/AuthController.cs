using Microsoft.AspNetCore.Mvc;
using Core.DTOs;
using Core.Interfaces;
using Microsoft.AspNetCore.Authorization;

namespace API.Controllers
{
    [ApiController]
    [Route("api/[controller]")]
    public class AuthController : ControllerBase
    {
        private readonly IUserService _userService;
        private readonly ITokenService _tokenService;

        public AuthController(IUserService userService, ITokenService tokenService)
        {
            _userService = userService;
            _tokenService = tokenService;
        }

        [HttpPost("register")]
        public IActionResult Register([FromBody] RegisterRequestDto request)
        {
            var user = _userService.Register(request);
            var token = _tokenService.GenerateToken(user.Username);

            return Ok(new RegisterResponseDto
            {
                Username = user.Username,
                Email = user.Email,
                Token = token
            });
        }

        [HttpPost("login")]
        public IActionResult Login([FromBody] LoginRequestDto request)
        {
            var user = _userService.Login(request);

            var token = _tokenService.GenerateToken(user.Username);

            return Ok(new LoginResponseDto
            {
                Username = user.Username,
                Token = token
            });
        }

        [Authorize]
        [HttpGet("me")]
        public IActionResult GetMe()
        {
            return Ok(new { Username = User?.Identity?.Name });
        }
    }
}
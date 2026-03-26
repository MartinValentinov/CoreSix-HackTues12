using Core.DTOs;

namespace Core.Interfaces
{
    public interface IUserService
    {
        LoginResponseDto Login(LoginRequestDto request);
        RegisterResponseDto Register(RegisterRequestDto request);
    }
}
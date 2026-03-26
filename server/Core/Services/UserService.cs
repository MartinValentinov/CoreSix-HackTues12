using Core.Interfaces;
using Core.DTOs;
using Core.Exceptions;
using Data.Entities;
using Data.Repositories;

namespace Core.Services
{
    public class UserService : IUserService
    {
        private readonly IUserRepository _repo;
        private readonly IPasswordHasher _hasher;

        public UserService(IUserRepository repo, IPasswordHasher hasher)
        {
            _repo = repo;
            _hasher = hasher;
        }

        public RegisterResponseDto Register(RegisterRequestDto request)
        {
            if (_repo.GetByUsername(request.Username) != null)
                throw new UserAlreadyExistsException(request.Username);

            var user = new User
            {
                Username = request.Username,
                Email = request.Email
            };

            user.PasswordHash = _hasher.HashPassword(user, request.Password);

            _repo.AddUser(user);
            _repo.SaveChanges();

            return new RegisterResponseDto
            {
                Username = user.Username,
                Email = user.Email,
            };
        }

        public LoginResponseDto Login(LoginRequestDto request)
        {
            var user = _repo.GetByUsername(request.Username)
                ?? throw new InvalidCredentialsException();

            if (!_hasher.VerifyPassword(user, request.Password))
                throw new InvalidCredentialsException();

            return new LoginResponseDto
            {
                Username = user.Username
            };
        }
    }
}
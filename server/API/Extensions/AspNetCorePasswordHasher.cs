using Core.Interfaces;
using Data.Entities;
using Microsoft.AspNetCore.Identity;

namespace API.Extensions
{   
    public class AspNetCorePasswordHasher : IPasswordHasher
    {
        private readonly PasswordHasher<User> _hasher = new();

        public string HashPassword(User user, string password) =>
            _hasher.HashPassword(user, password);

        public bool VerifyPassword(User user, string password) =>
            _hasher.VerifyHashedPassword(user, user.PasswordHash, password) == PasswordVerificationResult.Success;
    }
}
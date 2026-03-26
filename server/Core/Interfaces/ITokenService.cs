using System.Collections.Generic;

namespace Core.Interfaces
{
    public interface ITokenService
    {
        string GenerateToken(string username, int expireHours = 1);
    }
}
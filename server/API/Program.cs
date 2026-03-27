using Microsoft.EntityFrameworkCore;
using Data.Context;
using Data.Repositories;
using Core.Services;
using Core.Interfaces;
using API.Middleware;
using API.Extensions;
using API.Hubs;

using DotNetEnv;

var builder = WebApplication.CreateBuilder(args);

// Load environment variables from stable, content-root-based locations.
var contentRoot = builder.Environment.ContentRootPath;
var envCandidates = new[]
{
    Path.Combine(contentRoot, ".env"),
    Path.GetFullPath(Path.Combine(contentRoot, "..", ".env")),
    Path.GetFullPath(Path.Combine(contentRoot, "..", "Data", ".env"))
};

var envPath = envCandidates.FirstOrDefault(File.Exists);
if (!string.IsNullOrWhiteSpace(envPath))
{
    var envValues = ParseDotEnv(envPath);
    if (envValues.Count > 0)
    {
        builder.Configuration.AddInMemoryCollection(envValues);
    }

    Env.Load(envPath);
}

builder.Configuration.AddEnvironmentVariables();

builder.Services.AddSingleton<MongoDbContext>();
builder.Services.AddScoped<ChatService>();
builder.Services.AddScoped<IPasswordHasher, AspNetCorePasswordHasher>();
builder.Services.AddScoped<IUserRepository, UserRepository>();
builder.Services.AddScoped<IUserService, UserService>();
builder.Services.AddSingleton<ITokenService, TokenService>();

builder.Services.AddJwtAuth(builder.Configuration);
builder.Services.AddSignalR();
builder.Services.AddControllers();
builder.Services.AddCors(options =>
{
    options.AddPolicy("FrontendPolicy", policy =>
    {
        policy
            .WithOrigins("http://localhost:3000")
            .AllowAnyHeader()
            .AllowAnyMethod();
    });
});

var app = builder.Build();

using var scope = app.Services.CreateScope();
var dbContext = scope.ServiceProvider.GetRequiredService<MongoDbContext>();

app.UseMiddleware<LoggingMiddleware>();
app.UseMiddleware<ErrorHandlingMiddleware>();
app.UseCors("FrontendPolicy");
app.UseAuthentication();
app.UseAuthorization();
app.MapControllers();
app.MapHub<ChatHub>("/chat");
app.Run();

static Dictionary<string, string?> ParseDotEnv(string filePath)
{
    var result = new Dictionary<string, string?>(StringComparer.OrdinalIgnoreCase);

    foreach (var line in File.ReadLines(filePath))
    {
        var trimmed = line.Trim();
        if (string.IsNullOrWhiteSpace(trimmed) || trimmed.StartsWith("#"))
        {
            continue;
        }

        var separatorIndex = trimmed.IndexOf('=');
        if (separatorIndex <= 0)
        {
            continue;
        }

        var key = trimmed[..separatorIndex].Trim().TrimStart('\uFEFF');
        if (key.StartsWith("export ", StringComparison.OrdinalIgnoreCase))
        {
            key = key[7..].Trim();
        }

        var value = trimmed[(separatorIndex + 1)..].Trim();
        result[key] = value;
    }

    return result;
}
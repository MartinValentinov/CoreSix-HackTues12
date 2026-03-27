using Microsoft.AspNetCore.Http;

namespace API.Middleware
{
    public class LoggingMiddleware
    {
        private readonly RequestDelegate _next;

        public LoggingMiddleware(RequestDelegate next)
        {
            _next = next;
        }

        public async Task Invoke(HttpContext context)
        {
            Console.WriteLine($"[REQUEST] {context.Request.Method} {context.Request.Path}");
            try
            {
                await _next(context);
            }
            finally
            {
                Console.WriteLine($"[RESPONSE] {context.Response.StatusCode}");
            }
        }
    }
}
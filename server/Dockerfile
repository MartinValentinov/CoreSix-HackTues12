FROM mcr.microsoft.com/dotnet/sdk:10.0 AS build
WORKDIR /app

COPY API/*.csproj ./API/
COPY Core/*.csproj ./Core/
COPY Data/*.csproj ./Data/
RUN dotnet restore API/API.csproj

COPY . .

RUN dotnet publish API/API.csproj -c Release -o out

FROM mcr.microsoft.com/dotnet/aspnet:10.0
WORKDIR /app

COPY --from=build /app/out ./

ENV ASPNETCORE_URLS=http://+:5000

EXPOSE 5000
ENTRYPOINT ["dotnet", "API.dll"]
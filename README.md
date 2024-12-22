<h1 align="center"><strong>WEBSERV - @42SP</strong></h1>

<p align="center">Build your own HTTP server from scratch.</p>

<p align="center">
  <a href="https://www.42sp.org.br/" target="_blank">
    <img src="https://img.shields.io/static/v1?label=&message=SP&color=000&style=for-the-badge&logo=42">
  </a>
</p>
<p align="center">
  <img src="https://github.com/ayogun/42-project-badges/raw/main/badges/webservm.png">
</p>
<p align="center"><strong>Grade: 110/100</strong> ✔️</p>

---

<h2 align="center"><strong>What is Webserv?</strong></h2>

**Webserv** is a custom web server built from scratch as part of the 42SP curriculum. The project aims to deepen your understanding of the HTTP protocol and how web servers operate.  
The server can:  
- Handle multiple connections simultaneously.
- Serve static files (e.g., HTML, CSS, images).
- Process dynamic content using CGI (Common Gateway Interface).
- Support HTTP methods: **GET**, **POST**, and **DELETE**.

This project demonstrates skills in networking, concurrency, and HTTP communication.

---

<h2 align="center"><strong>Features</strong></h2>

- **Static File Serving**: Deliver files like HTML, CSS, JS, and images directly to clients.  
- **Dynamic Content with CGI**: Process dynamic scripts (Python, PHP) to generate responses.  
- **Multiple HTTP Methods**: 
  - **GET**: Retrieve resources.
  - **POST**: Send data to the server.
  - **DELETE**: Remove resources on the server.  
- **Concurrency**: Handle multiple client connections simultaneously.  
- **Configurable Server**: Customize ports, routes, and CGI settings using configuration files.

---

<h1 align="center"><strong></strong>How to use</h1>

- Clone the repository
- Run `make` to compile the library
- execute the program with `./webserver or ./webserver configs/(file)`

Acess the server with your browser in the configured port of the file.
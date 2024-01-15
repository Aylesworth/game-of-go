<a name="readme-top"></a>


<!-- PROJECT LOGO -->
<br />
<div align="center">

  <h3 align="center">Online Game Of Go Application</h3>

  <p align="center">
    An online platform where players can take part in Game of Go matches with others
    <br />
    <br />
    <a href="https://github.com/Aylesworth/game-of-go">View Demo</a>
    ·
    <a href="https://github.com/Aylesworth/game-of-go/issues">Report Bug</a>
    ·
    <a href="https://github.com/Aylesworth/game-of-go/issues">Request Feature</a>
  </p>
</div>



<!-- TABLE OF CONTENTS -->
<details>
  <summary>Table of Contents</summary>
  <ol>
    <li>
      <a href="#about-the-project">About The Project</a>
      <ul>
        <li><a href="#built-with">Built With</a></li>
      </ul>
    </li>
    <li>
      <a href="#getting-started">Getting Started</a>
      <ul>
        <li><a href="#prerequisites">Prerequisites</a></li>
        <li><a href="#installation">Installation</a></li>
      </ul>
    </li>
    <li><a href="#contact">Contact</a></li>
  </ol>
</details>



<!-- ABOUT THE PROJECT -->
## About The Project

[![Product Screen Shot][product-screenshot]]()

Go, also known as Weiqi in China and Baduk in Korea, is an ancient strategy board game that has captivated players for centuries. It provides an intricate blend of simplicity and complexity, making it an ideal choice for a networking programming project. The depth of Go strategy challenges programmers to create a sophisticated backend while the minimalistic nature of the game allows for an elegant user interface.

**Key Features**:
- ***Play with Friends***:
	- Challenge your friends to thrilling Go matches.
	- Seamless connectivity for real-time, engaging gameplay.
	- In-game chat to communicate and strategize with your opponents.
	
- ***Play against AI***:
	- Test your skills against a powerful AI opponent.
	
- ***Ranking System***:
	- Compete globally and locally on our dynamic leaderboards.
	- Earn ranks and climb the ladder by defeating opponents.
	- Regular updates ensure a fair and balanced ranking system.
	
- ***Historical Statistics***:
	- Dive into your gaming history with detailed match statistics.
	- Track progress over time and learn from each match.
	
- ***User-Friendly Interface***:
	- Intuitive design for a seamless user experience.
	- Effortless navigation between game modes and features.
	- Customizable settings to tailor the game to your preferences.
	

<p align="right">(<a href="#readme-top">back to top</a>)</p>



### Built With

* [![C++][Cpp]][Cpp-url]
* [![Qt][Qt.io]][Qt-url]
* [![MySQL][MySQL.com]][MySQL-url]

<p align="right">(<a href="#readme-top">back to top</a>)</p>



<!-- GETTING STARTED -->
## Getting Started

This application is currently confirmed to be compatible on [![Ubuntu 22.04][Ubuntu.com]][Ubuntu-url]
<br />

To get a local copy up and running follow these simple example steps.

### Prerequisites

* Build tools
  ```sh
  sudo apt update
  sudo apt install build-essential cmake
  ```

* MySQL database
  ```sh
  sudo apt install mysql-server
  ```

* C/C++ Libraries
  ```sh
  sudo apt install libmysqlcppconn-dev libssl-dev
  ```

* Qt framework
  ```sh 
  sudo apt install qtbase5-dev qt5-qmake
  ```
  

### Installation

1. Clone the repo
   ```sh
   git clone https://github.com/Aylesworth/game-of-go.git
   ```
2. Setup database

   - Create database
   ```sh
   sudo mysql
   mysql> create database game_of_go;
   ```
   - Import data
   ```sh
   sudo mysql game_of_go < game_of_go.sql
   ```
   - Create user and grant privileges
   ```sh
   sudo mysql
   mysql> create user 'your_username'@'localhost' identified by 'your_password';
   mysql> grant all privileges on game_of_go.* to 'your_username'@'localhost';
   mysql> flush privileges;
   ```
3. Start server
   - Change directory
   ```sh
   cd game-of-go-server
   ```
   - Config the database user in `dao.hpp`
   ```cpp
   con = driver->connect("tcp://localhost:3306", "your_username", "your_password");
   ```
   - Start server
   ```sh
   make clean all
   ./server
   ```
4. Start client application
   ```sh
   cd game-of-go-qt-ui/game-of-go-qt-ui/build
   qmake ../game-of-go-qt-ui.pro
   make
   ./game-of-go-qt-ui
   ```
   

<p align="right">(<a href="#readme-top">back to top</a>)</p>



<!-- CONTACT -->
## Contact

Nguyen Duc Anh - [Nguyễn Đức Anh](https://web.facebook.com/nda.2105) - nguyenducanh2105@gmail.com

Project Link: [https://github.com/Aylesworth/game-of-go](https://github.com/Aylesworth/game-of-go)

<p align="right">(<a href="#readme-top">back to top</a>)</p>



<!-- MARKDOWN LINKS & IMAGES -->
<!-- https://www.markdownguide.org/basic-syntax/#reference-style-links -->
[product-screenshot]: screenshots/screenshot-01.png
[Cpp]: https://img.shields.io/badge/C%2B%2B-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white
[Cpp-url]: https://isocpp.org/
[Qt.io]: https://img.shields.io/badge/Qt-%23217346.svg?style=for-the-badge&logo=Qt&logoColor=white
[Qt-url]: https://www.qt.io/
[MySQL.com]: https://img.shields.io/badge/MySQL-00000F?style=for-the-badge&logo=mysql&logoColor=white
[MySQL-url]: https://www.mysql.com/
[Ubuntu.com]: https://img.shields.io/badge/Ubuntu%2022.04-E95420?style=for-the-badge&logo=ubuntu&logoColor=white
[Ubuntu-url]: https://ubuntu.com/


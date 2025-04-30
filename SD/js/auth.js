// Функции авторизации и проверки сессии
document.addEventListener('DOMContentLoaded', function() {
    // Проверка, находимся ли мы на странице входа
    const isLoginPage = document.querySelector('.login-page') !== null;
    
    // Проверка авторизации
    checkAuth();
    
    // Обработчик кнопки входа
    if (isLoginPage) {
        const loginBtn = document.getElementById('login-btn');
        if (loginBtn) {
            loginBtn.addEventListener('click', login);
        }
        
        // Обработка нажатия Enter в полях ввода
        document.getElementById('username').addEventListener('keypress', function(e) {
            if (e.key === 'Enter') {
                document.getElementById('password').focus();
            }
        });
        
        document.getElementById('password').addEventListener('keypress', function(e) {
            if (e.key === 'Enter') {
                login();
            }
        });
    }
    
    // Обработчик кнопки выхода
    const logoutBtn = document.getElementById('logout-btn');
    if (logoutBtn) {
        logoutBtn.addEventListener('click', logout);
    }
    
    // Отображение имени пользователя
    const usernameDisplay = document.getElementById('username-display');
    if (usernameDisplay) {
        const username = localStorage.getItem('username');
        if (username) {
            usernameDisplay.textContent = username;
        }
    }
});

// Функция проверки авторизации
function checkAuth() {
    const isLoginPage = document.querySelector('.login-page') !== null;
    const token = localStorage.getItem('auth_token');
    
    if (!token && !isLoginPage) {
        // Перенаправление на страницу входа
        window.location.href = 'index.html';
    } else if (token && isLoginPage) {
        // Перенаправление на панель управления
        window.location.href = 'dashboard.html';
    }
}

// Функция входа
function login() {
    const username = document.getElementById('username').value;
    const password = document.getElementById('password').value;
    const errorElement = document.getElementById('login-error');
    
    if (!username || !password) {
        errorElement.textContent = 'Введите имя пользователя и пароль';
        return;
    }
    
    // Очистка сообщения об ошибке
    errorElement.textContent = '';
    
    // Создание базовой авторизации
    const base64Credentials = btoa(username + ':' + password);
    
    // Запрос к API
    fetch('/api', {
        method: 'GET',
        headers: {
            'Authorization': 'Basic ' + base64Credentials
        }
    })
    .then(response => {
        if (response.ok) {
            // Сохранение токена и имени пользователя
            localStorage.setItem('auth_token', base64Credentials);
            localStorage.setItem('username', username);
            
            // Перенаправление на панель управления
            window.location.href = 'dashboard.html';
        } else {
            throw new Error('Неверное имя пользователя или пароль');
        }
    })
    .catch(error => {
        errorElement.textContent = error.message;
    });
}

// Функция выхода
function logout() {
    // Удаление токена и имени пользователя
    localStorage.removeItem('auth_token');
    localStorage.removeItem('username');
    
    // Перенаправление на страницу входа
    window.location.href = 'index.html';
}

// Функция для выполнения API-запросов с авторизацией
function apiRequest(url, method = 'GET', data = null) {
    const token = localStorage.getItem('auth_token');
    
    const options = {
        method: method,
        headers: {
            'Authorization': 'Basic ' + token
        }
    };
    
    if (data) {
        if (method === 'POST' || method === 'PUT') {
            if (data instanceof FormData) {
                options.body = data;
            } else {
                options.headers['Content-Type'] = 'application/x-www-form-urlencoded';
                options.body = new URLSearchParams(data).toString();
            }
        }
    }
    
    return fetch(url, options)
        .then(response => {
            if (response.status === 401) {
                // Неавторизован, перенаправление на страницу входа
                logout();
                throw new Error('Сессия истекла. Пожалуйста, войдите снова.');
            }
            return response.json();
        });
}

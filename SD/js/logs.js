// Скрипт для страницы журнала событий
document.addEventListener('DOMContentLoaded', function() {
    // Загрузка журнала
    loadLogs();
    
    // Обработчики кнопок
    document.getElementById('refresh-logs-btn').addEventListener('click', function() {
        loadLogs();
    });
    
    document.getElementById('clear-logs-btn').addEventListener('click', function() {
        showClearLogsConfirmation();
    });
    
    // Обработчики фильтров
    document.getElementById('log-level-filter').addEventListener('change', function() {
        filterLogs();
    });
    
    document.getElementById('log-search').addEventListener('input', function() {
        filterLogs();
    });
    
    // Обработчики модального окна
    document.querySelector('.modal-close').addEventListener('click', function() {
        closeModal();
    });
    
    document.getElementById('confirm-cancel-btn').addEventListener('click', function() {
        closeModal();
    });
    
    document.getElementById('confirm-ok-btn').addEventListener('click', function() {
        clearLogs();
    });
    
    // Закрытие модального окна при клике вне его
    window.addEventListener('click', function(event) {
        const modal = document.getElementById('confirm-modal');
        if (event.target === modal) {
            closeModal();
        }
    });
});

// Глобальная переменная для хранения журнала
let logs = [];

// Загрузка журнала с сервера
function loadLogs() {
    apiRequest('/api/logs')
        .then(data => {
            logs = data.logs;
            renderLogs();
        })
        .catch(error => {
            console.error('Ошибка загрузки журнала:', error);
            alert('Ошибка загрузки журнала: ' + error.message);
        });
}

// Отображение журнала
function renderLogs() {
    const tableBody = document.getElementById('logs-table-body');
    tableBody.innerHTML = '';
    
    if (logs.length === 0) {
        const row = document.createElement('tr');
        row.innerHTML = '<td colspan="3" class="text-center">Журнал пуст</td>';
        tableBody.appendChild(row);
        return;
    }
    
    // Применение фильтров
    const levelFilter = document.getElementById('log-level-filter').value;
    const searchText = document.getElementById('log-search').value.toLowerCase();
    
    let filteredLogs = logs;
    
    // Фильтрация по уровню
    if (levelFilter !== 'all') {
        filteredLogs = filteredLogs.filter(log => log.level.toLowerCase() === levelFilter);
    }
    
    // Фильтрация по тексту
    if (searchText) {
        filteredLogs = filteredLogs.filter(log => 
            log.message.toLowerCase().includes(searchText) || 
            log.timestamp.toLowerCase().includes(searchText) ||
            log.level.toLowerCase().includes(searchText)
        );
    }
    
    if (filteredLogs.length === 0) {
        const row = document.createElement('tr');
        row.innerHTML = '<td colspan="3" class="text-center">Нет записей, соответствующих фильтру</td>';
        tableBody.appendChild(row);
        return;
    }
    
    // Отображение отфильтрованных записей
    filteredLogs.forEach(log => {
        const row = document.createElement('tr');
        
        const timeCell = document.createElement('td');
        timeCell.textContent = log.timestamp;
        
        const levelCell = document.createElement('td');
        const levelSpan = document.createElement('span');
        levelSpan.className = 'log-level-' + log.level.toLowerCase();
        levelSpan.textContent = log.level;
        levelCell.appendChild(levelSpan);
        
        const messageCell = document.createElement('td');
        messageCell.textContent = log.message;
        
        row.appendChild(timeCell);
        row.appendChild(levelCell);
        row.appendChild(messageCell);
        
        tableBody.appendChild(row);
    });
}

// Фильтрация журнала
function filterLogs() {
    renderLogs();
}

// Отображение подтверждения очистки журнала
function showClearLogsConfirmation() {
    document.getElementById('confirm-modal').style.display = 'block';
}

// Закрытие модального окна
function closeModal() {
    document.getElementById('confirm-modal').style.display = 'none';
}

// Очистка журнала
function clearLogs() {
    apiRequest('/api/logs/clear', 'POST')
        .then(data => {
            if (data.success) {
                logs = [];
                renderLogs();
                alert('Журнал успешно очищен');
            } else {
                throw new Error(data.error || 'Ошибка очистки журнала');
            }
        })
        .catch(error => {
            console.error('Ошибка очистки журнала:', error);
            alert('Ошибка очистки журнала: ' + error.message);
        });
}

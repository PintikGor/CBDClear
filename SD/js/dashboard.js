// Скрипт для панели управления
document.addEventListener('DOMContentLoaded', function() {
    // Загрузка данных
    loadDashboardData();
    
    // Обновление данных каждые 5 секунд
    setInterval(loadDashboardData, 5000);
    
    // Обновление времени каждую секунду
    setInterval(updateCurrentTime, 1000);
    
    // Обработчики кнопок управления нагрузками
    document.getElementById('load1-on').addEventListener('click', function() {
        toggleLoad(1, true);
    });
    
    document.getElementById('load1-off').addEventListener('click', function() {
        toggleLoad(1, false);
    });
    
    document.getElementById('load2-on').addEventListener('click', function() {
        toggleLoad(2, true);
    });
    
    document.getElementById('load2-off').addEventListener('click', function() {
        toggleLoad(2, false);
    });
});

// Загрузка данных для панели управления
function loadDashboardData() {
    apiRequest('/api')
        .then(data => {
            // Обновление версии прошивки
            document.getElementById('firmware-version').textContent = data.version;
            
            // Обновление температуры
            document.getElementById('temperature').textContent = data.temperature + ' °C';
            
            // Обновление уровня воды
            updateWaterLevel(data.waterLevel);
            
            // Обновление статуса нагрузок
            updateLoadStatus(1, data.load1);
            updateLoadStatus(2, data.load2);
            
            // Обновление информации о следующем событии
            document.getElementById('next-schedule').textContent = data.nextSchedule;
        })
        .catch(error => {
            console.error('Ошибка загрузки данных:', error);
            document.getElementById('device-status').textContent = 'Офлайн';
            document.getElementById('device-status').className = 'status-value status-offline';
        });
    
    // Загрузка системной информации
    apiRequest('/api/system')
        .then(data => {
            // Обновление времени работы
            const uptime = formatUptime(data.uptime);
            document.getElementById('uptime').textContent = uptime;
            
            // Обновление информации о SD-карте
            const sdCardInfo = `${data.sdCardUsed} МБ / ${data.sdCardSize} МБ`;
            document.getElementById('sd-card-info').textContent = sdCardInfo;
            
            // Обновление информации о свободной памяти
            document.getElementById('free-memory').textContent = `${data.freeHeap} байт`;
            
            // Обновление IP-адреса
            const ip = localStorage.getItem('device_ip') || 'Неизвестно';
            document.getElementById('ip-address').textContent = ip;
        })
        .catch(error => {
            console.error('Ошибка загрузки системной информации:', error);
        });
}

// Обновление текущего времени
function updateCurrentTime() {
    const now = new Date();
    const options = { 
        weekday: 'long', 
        year: 'numeric', 
        month: 'long', 
        day: 'numeric',
        hour: '2-digit',
        minute: '2-digit',
        second: '2-digit'
    };
    
    document.getElementById('current-time').textContent = now.toLocaleDateString('ru-RU', options);
}

// Обновление уровня воды
function updateWaterLevel(level) {
    const waterLevelFill = document.getElementById('water-level-fill');
    const waterLevelStatus = document.getElementById('water-level-status');
    
    let fillHeight = '0%';
    let statusText = 'Пустой';
    let statusClass = 'status-value status-error';
    
    if (level === 'empty') {
        fillHeight = '0%';
        statusText = 'Пустой';
        statusClass = 'status-value status-error';
    } else if (level === 'normal') {
        fillHeight = '50%';
        statusText = 'Нормальный';
        statusClass = 'status-value status-warning';
    } else if (level === 'full') {
        fillHeight = '100%';
        statusText = 'Полный';
        statusClass = 'status-value status-online';
    }
    
    waterLevelFill.style.height = fillHeight;
    waterLevelStatus.textContent = statusText;
    waterLevelStatus.className = statusClass;
}

// Обновление статуса нагрузки
function updateLoadStatus(loadNumber, status) {
    const statusElement = document.getElementById(`load${loadNumber}-status`);
    
    if (status) {
        statusElement.textContent = 'Включен';
        statusElement.className = 'status-value status-on';
    } else {
        statusElement.textContent = 'Выключен';
        statusElement.className = 'status-value status-off';
    }
}

// Переключение нагрузки
function toggleLoad(loadNumber, state) {
    apiRequest('/api/toggle', 'POST', {
        load: loadNumber,
        state: state ? 1 : 0
    })
    .then(data => {
        if (data.success) {
            updateLoadStatus(loadNumber, data.state);
        }
    })
    .catch(error => {
        console.error('Ошибка переключения нагрузки:', error);
        alert('Ошибка переключения нагрузки: ' + error.message);
    });
}

// Форматирование времени работы
function formatUptime(seconds) {
    const days = Math.floor(seconds / 86400);
    seconds %= 86400;
    const hours = Math.floor(seconds / 3600);
    seconds %= 3600;
    const minutes = Math.floor(seconds / 60);
    seconds %= 60;
    
    let result = '';
    if (days > 0) {
        result += days + ' д. ';
    }
    
    return result + 
        String(hours).padStart(2, '0') + ':' + 
        String(minutes).padStart(2, '0') + ':' + 
        String(seconds).padStart(2, '0');
}

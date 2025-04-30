// Скрипт для страницы расписания
document.addEventListener('DOMContentLoaded', function() {
    // Загрузка расписаний
    loadSchedules();
    
    // Обработчики кнопок
    document.getElementById('add-schedule-btn').addEventListener('click', function() {
        openScheduleModal();
    });
    
    document.getElementById('save-schedules-btn').addEventListener('click', function() {
        saveAllSchedules();
    });
    
    document.getElementById('schedule-save-btn').addEventListener('click', function() {
        saveSchedule();
    });
    
    document.getElementById('schedule-cancel-btn').addEventListener('click', function() {
        closeScheduleModal();
    });
    
    // Закрытие модального окна при клике на крестик
    document.querySelector('.modal-close').addEventListener('click', function() {
        closeScheduleModal();
    });
    
    // Изменение режима расписания
    document.getElementById('schedule-mode').addEventListener('change', function() {
        updateModeFields();
    });
    
    // Изменение типа окончания
    document.getElementById('schedule-use-duration').addEventListener('change', function() {
        updateEndTimeFields();
    });
    
    // Закрытие модального окна при клике вне его
    window.addEventListener('click', function(event) {
        const modal = document.getElementById('schedule-modal');
        if (event.target === modal) {
            closeScheduleModal();
        }
    });
});

// Глобальная переменная для хранения расписаний
let schedules = [];

// Загрузка расписаний с сервера
function loadSchedules() {
    apiRequest('/api/schedule')
        .then(data => {
            schedules = data.schedules;
            renderScheduleTable();
        })
        .catch(error => {
            console.error('Ошибка загрузки расписаний:', error);
            alert('Ошибка загрузки расписаний: ' + error.message);
        });
}

// Отображение таблицы расписаний
function renderScheduleTable() {
    const tableBody = document.getElementById('schedule-list-body');
    tableBody.innerHTML = '';
    
    if (schedules.length === 0) {
        const row = document.createElement('tr');
        row.innerHTML = '<td colspan="7" class="text-center">Нет настроенных расписаний</td>';
        tableBody.appendChild(row);
        return;
    }
    
    schedules.forEach((schedule, index) => {
        const row = document.createElement('tr');
        
        // Статус
        const statusCell = document.createElement('td');
        const statusLabel = document.createElement('span');
        statusLabel.className = schedule.enabled ? 'status-on' : 'status-off';
        statusLabel.textContent = schedule.enabled ? 'Включено' : 'Выключено';
        statusCell.appendChild(statusLabel);
        
        // Нагрузка
        const loadCell = document.createElement('td');
        loadCell.textContent = `Нагрузка ${schedule.loadNumber}`;
        
        // Режим
        const modeCell = document.createElement('td');
        let modeText = '';
        switch (schedule.mode) {
            case 0: modeText = 'Ежедневно'; break;
            case 1: modeText = 'По дням недели'; break;
            case 2: modeText = 'Ежемесячно'; break;
        }
        modeCell.textContent = modeText;
        
        // Дни
        const daysCell = document.createElement('td');
        if (schedule.mode === 0) {
            daysCell.textContent = 'Каждый день';
        } else if (schedule.mode === 1) {
            const days = [];
            const dayNames = ['Вс', 'Пн', 'Вт', 'Ср', 'Чт', 'Пт', 'Сб'];
            for (let i = 0; i < 7; i++) {
                if ((schedule.days & (1 << i)) !== 0) {
                    days.push(dayNames[i]);
                }
            }
            daysCell.textContent = days.join(', ');
        } else if (schedule.mode === 2) {
            daysCell.textContent = `${schedule.day}-е число`;
        }
        
        // Время включения
        const startTimeCell = document.createElement('td');
        startTimeCell.textContent = formatTime(schedule.hour, schedule.minute, schedule.second);
        
        // Длительность / Время выключения
        const endTimeCell = document.createElement('td');
        if (schedule.useDuration) {
            endTimeCell.textContent = `${schedule.durationHour}ч ${schedule.durationMinute}м ${schedule.durationSecond}с`;
        } else {
            endTimeCell.textContent = formatTime(schedule.endHour, schedule.endMinute, schedule.endSecond);
        }
        
        // Действия
        const actionsCell = document.createElement('td');
        
        const editBtn = document.createElement('button');
        editBtn.className = 'btn btn-primary btn-sm';
        editBtn.textContent = 'Изменить';
        editBtn.addEventListener('click', function() {
            openScheduleModal(index);
        });
        
        const deleteBtn = document.createElement('button');
        deleteBtn.className = 'btn btn-danger btn-sm';
        deleteBtn.textContent = 'Удалить';
        deleteBtn.addEventListener('click', function() {
            if (confirm('Вы уверены, что хотите удалить это расписание?')) {
                deleteSchedule(index);
            }
        });
        
        actionsCell.appendChild(editBtn);
        actionsCell.appendChild(document.createTextNode(' '));
        actionsCell.appendChild(deleteBtn);
        
        // Добавление ячеек в строку
        row.appendChild(statusCell);
        row.appendChild(loadCell);
        row.appendChild(modeCell);
        row.appendChild(daysCell);
        row.appendChild(startTimeCell);
        row.appendChild(endTimeCell);
        row.appendChild(actionsCell);
        
        tableBody.appendChild(row);
    });
}

// Открытие модального окна для добавления/редактирования расписания
function openScheduleModal(index = -1) {
    const modal = document.getElementById('schedule-modal');
    const modalTitle = document.getElementById('schedule-modal-title');
    
    // Очистка формы
    document.getElementById('schedule-form').reset();
    
    if (index >= 0) {
        // Редактирование существующего расписания
        modalTitle.textContent = 'Изменить расписание';
        
        const schedule = schedules[index];
        document.getElementById('schedule-id').value = index;
        document.getElementById('schedule-enabled').checked = schedule.enabled;
        document.getElementById('schedule-load').value = schedule.loadNumber;
        document.getElementById('schedule-mode').value = schedule.mode;
        
        // Дни недели
        if (schedule.mode === 1) {
            for (let i = 0; i < 7; i++) {
                document.getElementById(`day-${['sun', 'mon', 'tue', 'wed', 'thu', 'fri', 'sat'][i]}`).checked = 
                    (schedule.days & (1 << i)) !== 0;
            }
        }
        
        // День месяца
        document.getElementById('schedule-day').value = schedule.day;
        
        // Время включения
        document.getElementById('schedule-hour').value = schedule.hour;
        document.getElementById('schedule-minute').value = schedule.minute;
        document.getElementById('schedule-second').value = schedule.second;
        
        // Тип окончания
        document.getElementById('schedule-use-duration').checked = schedule.useDuration;
        
        // Длительность
        document.getElementById('schedule-duration-hour').value = schedule.durationHour;
        document.getElementById('schedule-duration-minute').value = schedule.durationMinute;
        document.getElementById('schedule-duration-second').value = schedule.durationSecond;
        
        // Время выключения
        document.getElementById('schedule-end-hour').value = schedule.endHour;
        document.getElementById('schedule-end-minute').value = schedule.endMinute;
        document.getElementById('schedule-end-second').value = schedule.endSecond;
    } else {
        // Добавление нового расписания
        modalTitle.textContent = 'Добавить расписание';
        document.getElementById('schedule-id').value = -1;
    }
        // Обновление отображения полей в зависимости от режима
    updateModeFields();
    updateEndTimeFields();
    
    // Отображение модального окна
    modal.style.display = 'block';
}

// Закрытие модального окна
function closeScheduleModal() {
    document.getElementById('schedule-modal').style.display = 'none';
}

// Обновление полей в зависимости от выбранного режима
function updateModeFields() {
    const mode = parseInt(document.getElementById('schedule-mode').value);
    
    // Скрытие/отображение полей для дней недели
    document.getElementById('schedule-days-weekly').style.display = mode === 1 ? 'block' : 'none';
    
    // Скрытие/отображение полей для дня месяца
    document.getElementById('schedule-days-monthly').style.display = mode === 2 ? 'block' : 'none';
}

// Обновление полей в зависимости от типа окончания
function updateEndTimeFields() {
    const useDuration = document.getElementById('schedule-use-duration').checked;
    
    // Скрытие/отображение полей для длительности
    document.getElementById('schedule-duration').style.display = useDuration ? 'block' : 'none';
    
    // Скрытие/отображение полей для времени выключения
    document.getElementById('schedule-end-time').style.display = useDuration ? 'none' : 'block';
}

// Сохранение расписания
function saveSchedule() {
    const id = parseInt(document.getElementById('schedule-id').value);
    const enabled = document.getElementById('schedule-enabled').checked;
    const loadNumber = parseInt(document.getElementById('schedule-load').value);
    const mode = parseInt(document.getElementById('schedule-mode').value);
    
    // Получение дней недели
    let days = 0;
    if (mode === 1) {
        const dayIds = ['sun', 'mon', 'tue', 'wed', 'thu', 'fri', 'sat'];
        for (let i = 0; i < 7; i++) {
            if (document.getElementById(`day-${dayIds[i]}`).checked) {
                days |= (1 << i);
            }
        }
    } else {
        days = 0x7F; // Все дни недели
    }
    
    // Получение дня месяца
    const day = parseInt(document.getElementById('schedule-day').value);
    
    // Получение времени включения
    const hour = parseInt(document.getElementById('schedule-hour').value);
    const minute = parseInt(document.getElementById('schedule-minute').value);
    const second = parseInt(document.getElementById('schedule-second').value);
    
    // Получение типа окончания
    const useDuration = document.getElementById('schedule-use-duration').checked;
    
    // Получение длительности
    const durationHour = parseInt(document.getElementById('schedule-duration-hour').value);
    const durationMinute = parseInt(document.getElementById('schedule-duration-minute').value);
    const durationSecond = parseInt(document.getElementById('schedule-duration-second').value);
    
    // Получение времени выключения
    const endHour = parseInt(document.getElementById('schedule-end-hour').value);
    const endMinute = parseInt(document.getElementById('schedule-end-minute').value);
    const endSecond = parseInt(document.getElementById('schedule-end-second').value);
    
    // Создание объекта расписания
    const schedule = {
        id: id >= 0 ? id : schedules.length,
        enabled: enabled,
        loadNumber: loadNumber,
        mode: mode,
        days: days,
        day: day,
        hour: hour,
        minute: minute,
        second: second,
        useDuration: useDuration,
        durationHour: durationHour,
        durationMinute: durationMinute,
        durationSecond: durationSecond,
        endHour: endHour,
        endMinute: endMinute,
        endSecond: endSecond
    };
    
    // Добавление или обновление расписания
    if (id >= 0) {
        schedules[id] = schedule;
    } else {
        schedules.push(schedule);
    }
    
    // Обновление таблицы
    renderScheduleTable();
    
    // Закрытие модального окна
    closeScheduleModal();
}

// Удаление расписания
function deleteSchedule(index) {
    schedules.splice(index, 1);
    renderScheduleTable();
}

// Сохранение всех расписаний на сервер
function saveAllSchedules() {
    apiRequest('/api/schedule', 'POST', {
        data: JSON.stringify({ schedules: schedules })
    })
    .then(data => {
        if (data.success) {
            alert('Расписания успешно сохранены');
        } else {
            throw new Error(data.error || 'Ошибка сохранения расписаний');
        }
    })
    .catch(error => {
        console.error('Ошибка сохранения расписаний:', error);
        alert('Ошибка сохранения расписаний: ' + error.message);
    });
}

// Форматирование времени
function formatTime(hours, minutes, seconds) {
    return `${String(hours).padStart(2, '0')}:${String(minutes).padStart(2, '0')}:${String(seconds).padStart(2, '0')}`;
}

    

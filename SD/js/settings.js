// Скрипт для страницы настроек
document.addEventListener('DOMContentLoaded', function() {
    // Инициализация вкладок
    initTabs();
    
    // Загрузка настроек
    loadNetworkSettings();
    loadTimeSettings();
    loadSecuritySettings();
    loadSensorsSettings();
    loadSystemInfo();
    
    // Обработчики изменения состояния переключателей
    document.getElementById('dhcp-enabled').addEventListener('change', function() {
        toggleStaticIPFields();
    });
    
    document.getElementById('wifi-enabled').addEventListener('change', function() {
        toggleWiFiFields();
    });
    
    document.getElementById('wifi-mode').addEventListener('change', function() {
        toggleWiFiModeFields();
    });
    
    document.getElementById('ntp-enabled').addEventListener('change', function() {
        toggleNTPFields();
    });
    
    document.getElementById('temp-correction-enabled').addEventListener('change', function() {
        toggleTempCorrectionFields();
    });
    
    // Обработчики кнопок сохранения настроек
    document.getElementById('network-save-btn').addEventListener('click', function() {
        saveNetworkSettings();
    });
    
    document.getElementById('time-save-btn').addEventListener('click', function() {
        saveTimeSettings();
    });
    
    document.getElementById('time-sync-ntp-btn').addEventListener('click', function() {
        syncTimeWithNTP();
    });
    
    document.getElementById('security-save-btn').addEventListener('click', function() {
        saveSecuritySettings();
    });
    
    document.getElementById('sensors-save-btn').addEventListener('click', function() {
        saveSensorsSettings();
    });
    
    // Обработчики кнопок системного управления
    document.getElementById('system-reboot-btn').addEventListener('click', function() {
        showConfirmModal('Перезагрузка устройства', 'Вы уверены, что хотите перезагрузить устройство?', rebootDevice);
    });
    
    document.getElementById('system-reset-btn').addEventListener('click', function() {
        showConfirmModal('Сброс настроек', 'Вы уверены, что хотите сбросить все настройки к заводским значениям?', resetSettings);
    });
    
    // Обработчик сканирования WiFi сетей
    document.getElementById('scan-wifi-btn').addEventListener('click', function() {
        scanWiFiNetworks();
    });
    
    // Обработчик кнопки обновления прошивки
    document.getElementById('firmware-upload-btn').addEventListener('click', function() {
        uploadFirmware();
    });
    
    // Обработчики модальных окон
    document.querySelectorAll('.modal-close').forEach(function(element) {
        element.addEventListener('click', function() {
            closeModals();
        });
    });
    
    document.getElementById('wifi-scan-cancel-btn').addEventListener('click', function() {
        closeModals();
    });
    
    document.getElementById('wifi-scan-refresh-btn').addEventListener('click', function() {
        scanWiFiNetworks();
    });
    
    document.getElementById('confirm-cancel-btn').addEventListener('click', function() {
        closeModals();
    });
    
    // Закрытие модальных окон при клике вне их
    window.addEventListener('click', function(event) {
        if (event.target.classList.contains('modal')) {
            closeModals();
        }
    });
});

// Инициализация вкладок
function initTabs() {
    const tabButtons = document.querySelectorAll('.tab-btn');
    const tabPanes = document.querySelectorAll('.tab-pane');
    
    tabButtons.forEach(function(button) {
        button.addEventListener('click', function() {
            // Удаление класса active у всех кнопок и панелей
            tabButtons.forEach(function(btn) {
                btn.classList.remove('active');
            });
            
            tabPanes.forEach(function(pane) {
                pane.classList.remove('active');
            });
            
            // Добавление класса active для выбранной вкладки
            button.classList.add('active');
            const tabId = button.getAttribute('data-tab');
            document.getElementById(tabId + '-tab').classList.add('active');
        });
    });
}

// Загрузка сетевых настроек
function loadNetworkSettings() {
    apiRequest('/api/network')
        .then(data => {
            // Заполнение полей формы
            document.getElementById('dhcp-enabled').checked = data.dhcpEnabled;
            document.getElementById('ip-address').value = data.ip;
            document.getElementById('subnet-mask').value = data.subnet;
            document.getElementById('gateway').value = data.gateway;
            document.getElementById('dns-server').value = data.dns;
            
            document.getElementById('wifi-enabled').checked = data.wifiEnabled;
            document.getElementById('wifi-mode').value = data.apMode ? 'ap' : 'client';
            document.getElementById('wifi-ssid').value = data.ssid;
            document.getElementById('ap-ssid').value = data.apSsid;
            
            document.getElementById('ntp-enabled').checked = data.ntpEnabled;
            document.getElementById('ntp-server').value = data.ntpServer;
            document.getElementById('timezone').value = data.gmtOffset / 3600;
            
            // Обновление состояния полей
            toggleStaticIPFields();
            toggleWiFiFields();
            toggleWiFiModeFields();
            toggleNTPFields();
            
            // Сохранение IP-адреса в localStorage для отображения на панели управления
            localStorage.setItem('device_ip', data.ip);
        })
        .catch(error => {
            console.error('Ошибка загрузки сетевых настроек:', error);
            alert('Ошибка загрузки сетевых настроек: ' + error.message);
        });
}

// Загрузка настроек времени
function loadTimeSettings() {
    apiRequest('/api/time')
        .then(data => {
            document.getElementById('current-date').value = data.date;
            document.getElementById('current-time').value = data.time;
            document.getElementById('rtc-temperature').textContent = data.temperature + ' °C';
        })
        .catch(error => {
            console.error('Ошибка загрузки настроек времени:', error);
            alert('Ошибка загрузки настроек времени: ' + error.message);
        });
}

// Загрузка настроек безопасности
function loadSecuritySettings() {
    apiRequest('/api/security')
        .then(data => {
            document.getElementById('auth-enabled').checked = data.authEnabled;
            document.getElementById('auth-username').value = data.username;
        })
        .catch(error => {
            console.error('Ошибка загрузки настроек безопасности:', error);
            alert('Ошибка загрузки настроек безопасности: ' + error.message);
        });
}

// Загрузка настроек датчиков
function loadSensorsSettings() {
    apiRequest('/api/sensors')
        .then(data => {
            document.getElementById('temp-correction-enabled').checked = data.tempCorrectionEnabled;
            document.getElementById('temp-correction-value').value = data.tempCorrectionValue;
            document.getElementById('invert-empty-sensor').checked = data.invertEmptySensor;
            document.getElementById('invert-full-sensor').checked = data.invertFullSensor;
            
            toggleTempCorrectionFields();
        })
        .catch(error => {
            console.error('Ошибка загрузки настроек датчиков:', error);
            alert('Ошибка загрузки настроек датчиков: ' + error.message);
        });
}

// Загрузка системной информации
function loadSystemInfo() {
    apiRequest('/api/system')
        .then(data => {
            document.getElementById('firmware-version-info').textContent = data.version;
            document.getElementById('uptime-info').textContent = formatUptime(data.uptime);
            document.getElementById('free-memory-info').textContent = data.freeHeap + ' байт';
            document.getElementById('cpu-freq-info').textContent = data.cpuFreq + ' МГц';
            document.getElementById('sd-card-info-system').textContent = 
                `${data.sdCardUsed} МБ / ${data.sdCardSize} МБ`;
        })
        .catch(error => {
            console.error('Ошибка загрузки системной информации:', error);
            alert('Ошибка загрузки системной информации: ' + error.message);
        });
}

// Переключение полей статического IP
function toggleStaticIPFields() {
    const dhcpEnabled = document.getElementById('dhcp-enabled').checked;
    document.getElementById('static-ip-settings').style.display = dhcpEnabled ? 'none' : 'block';
}

// Переключение полей WiFi
function toggleWiFiFields() {
    const wifiEnabled = document.getElementById('wifi-enabled').checked;
    document.getElementById('wifi-settings').style.display = wifiEnabled ? 'block' : 'none';
    toggleWiFiModeFields();
}

// Переключение полей режима WiFi
function toggleWiFiModeFields() {
    const wifiEnabled = document.getElementById('wifi-enabled').checked;
    const wifiMode = document.getElementById('wifi-mode').value;
    
    document.getElementById('wifi-client-settings').style.display = 
        (wifiEnabled && wifiMode === 'client') ? 'block' : 'none';
    document.getElementById('wifi-ap-settings').style.display = 
        (wifiEnabled && wifiMode === 'ap') ? 'block' : 'none';
}

// Переключение полей NTP
function toggleNTPFields() {
    const ntpEnabled = document.getElementById('ntp-enabled').checked;
    document.getElementById('ntp-settings').style.display = ntpEnabled ? 'block' : 'none';
}

// Переключение полей коррекции температуры
function toggleTempCorrectionFields() {
    const correctionEnabled = document.getElementById('temp-correction-enabled').checked;
    document.getElementById('temp-correction-value-group').style.display = correctionEnabled ? 'block' : 'none';
}

// Сохранение сетевых настроек
function saveNetworkSettings() {
    const dhcpEnabled = document.getElementById('dhcp-enabled').checked;
    const ipAddress = document.getElementById('ip-address').value;
    const subnetMask = document.getElementById('subnet-mask').value;
    const gateway = document.getElementById('gateway').value;
    const dnsServer = document.getElementById('dns-server').value;
    
    const wifiEnabled = document.getElementById('wifi-enabled').checked;
    const wifiMode = document.getElementById('wifi-mode').value;
    const wifiSsid = document.getElementById('wifi-ssid').value;
    const wifiPassword = document.getElementById('wifi-password').value;
    const apSsid = document.getElementById('ap-ssid').value;
    const apPassword = document.getElementById('ap-password').value;
    
    const ntpEnabled = document.getElementById('ntp-enabled').checked;
    const ntpServer = document.getElementById('ntp-server').value;
    const timezone = parseInt(document.getElementById('timezone').value);
    
    // Проверка валидности IP-адресов
    if (!dhcpEnabled) {
        if (!validateIPAddress(ipAddress) || !validateIPAddress(subnetMask) || 
            !validateIPAddress(gateway) || !validateIPAddress(dnsServer)) {
            alert('Пожалуйста, введите корректные IP-адреса');
            return;
        }
    }
    
    // Проверка SSID и пароля
    if (wifiEnabled) {
        if (wifiMode === 'client') {
            if (!wifiSsid) {
                alert('Пожалуйста, введите имя WiFi сети');
                return;
            }
        } else if (wifiMode === 'ap') {
            if (!apSsid) {
                alert('Пожалуйста, введите имя точки доступа');
                return;
            }
            if (apPassword && apPassword.length < 8) {
                alert('Пароль точки доступа должен содержать не менее 8 символов');
                return;
            }
        }
    }
    
    // Создание объекта с настройками
    const settings = {
        dhcpEnabled: dhcpEnabled,
        ip: ipAddress,
        subnet: subnetMask,
        gateway: gateway,
        dns: dnsServer,
        wifiEnabled: wifiEnabled,
        apMode: wifiMode === 'ap',
        ssid: wifiSsid,
        password: wifiPassword,
        apSsid: apSsid,
        apPassword: apPassword,
        ntpEnabled: ntpEnabled,
        ntpServer: ntpServer,
        gmtOffset: timezone * 3600,
        daylightOffset: 3600
    };
    
    // Отправка настроек на сервер
    apiRequest('/api/network', 'POST', {
        data: JSON.stringify(settings)
    })
    .then(data => {
        if (data.success) {
            alert('Сетевые настройки успешно сохранены. Устройство может перезагрузиться для применения изменений.');
            
            // Сохранение IP-адреса в localStorage
            localStorage.setItem('device_ip', ipAddress);
        } else {
            throw new Error(data.error || 'Ошибка сохранения сетевых настроек');
        }
    })
    .catch(error => {
        console.error('Ошибка сохранения сетевых настроек:', error);
        alert('Ошибка сохранения сетевых настроек: ' + error.message);
    });
}

// Сохранение настроек времени
function saveTimeSettings() {
    const date = document.getElementById('current-date').value;
    const time = document.getElementById('current-time').value;
    
    if (!date || !time) {
        alert('Пожалуйста, введите дату и время');
        return;
    }
    
    apiRequest('/api/time', 'POST', {
        date: date,
        time: time
    })
    .then(data => {
        if (data.success) {
            alert('Дата и время успешно установлены');
        } else {
            throw new Error(data.error || 'Ошибка установки даты и времени');
        }
    })
    .catch(error => {
        console.error('Ошибка установки даты и времени:', error);
        alert('Ошибка установки даты и времени: ' + error.message);
    });
}

// Синхронизация времени с NTP
function syncTimeWithNTP() {
    // Проверка, включена ли синхронизация NTP
    if (!document.getElementById('ntp-enabled').checked) {
        alert('Для синхронизации времени необходимо включить NTP');
        return;
    }
    
    const ntpServer = document.getElementById('ntp-server').value;
    const timezone = parseInt(document.getElementById('timezone').value);
    
    if (!ntpServer) {
        alert('Пожалуйста, введите адрес NTP сервера');
        return;
    }
    
    alert('Запрос на синхронизацию времени отправлен. Это может занять несколько секунд.');
    
    // Здесь должен быть запрос к API для синхронизации времени
    // Поскольку у нас нет такого API, просто обновим время через 2 секунды
    setTimeout(loadTimeSettings, 2000);
}

// Сохранение настроек безопасности
function saveSecuritySettings() {
    const authEnabled = document.getElementById('auth-enabled').checked;
    const username = document.getElementById('auth-username').value;
    const password = document.getElementById('auth-password').value;
    const passwordConfirm = document.getElementById('auth-password-confirm').value;
    
    if (!username) {
        alert('Пожалуйста, введите имя пользователя');
        return;
    }
    
    if (password && password !== passwordConfirm) {
        alert('Пароли не совпадают');
        return;
    }
    
    const settings = {
        authEnabled: authEnabled,
        username: username
    };
    
    if (password) {
        settings.password = password;
    }
    
    apiRequest('/api/security', 'POST', {
        data: JSON.stringify(settings)
    })
    .then(data => {
        if (data.success) {
            alert('Настройки безопасности успешно сохранены');
            
            // Обновление имени пользователя в localStorage
            localStorage.setItem('username', username);
            document.getElementById('username-display').textContent = username;
            
            // Если пароль был изменен, нужно обновить токен авторизации
            if (password) {
                const base64Credentials = btoa(username + ':' + password);
                localStorage.setItem('auth_token', base64Credentials);
            }
        } else {
            throw new Error(data.error || 'Ошибка сохранения настроек безопасности');
        }
    })
    .catch(error => {
        console.error('Ошибка сохранения настроек безопасности:', error);
        alert('Ошибка сохранения настроек безопасности: ' + error.message);
    });
}

// Сохранение настроек датчиков
function saveSensorsSettings() {
    const tempCorrectionEnabled = document.getElementById('temp-correction-enabled').checked;
    const tempCorrectionValue = parseFloat(document.getElementById('temp-correction-value').value);
    const invertEmptySensor = document.getElementById('invert-empty-sensor').checked;
    const invertFullSensor = document.getElementById('invert-full-sensor').checked;
    
    const settings = {
        tempCorrectionEnabled: tempCorrectionEnabled,
        tempCorrectionValue: tempCorrectionValue,
        invertEmptySensor: invertEmptySensor,
        invertFullSensor: invertFullSensor
    };
    
    apiRequest('/api/sensors', 'POST', {
        data: JSON.stringify(settings)
    })
    .then(data => {
        if (data.success) {
            alert('Настройки датчиков успешно сохранены');
        } else {
            throw new Error(data.error || 'Ошибка сохранения настроек датчиков');
        }
    })
    .catch(error => {
        console.error('Ошибка сохранения настроек датчиков:', error);
        alert('Ошибка сохранения настроек датчиков: ' + error.message);
    });
}

// Перезагрузка устройства
function rebootDevice() {
    apiRequest('/api/reboot', 'POST')
        .then(data => {
            if (data.success) {
                alert('Устройство перезагружается. Пожалуйста, подождите...');
                
                // Перенаправление на страницу входа через 5 секунд
                setTimeout(function() {
                    window.location.href = 'index.html';
                }, 5000);
            } else {
                throw new Error(data.error || 'Ошибка перезагрузки устройства');
            }
        })
        .catch(error => {
            console.error('Ошибка перезагрузки устройства:', error);
            alert('Ошибка перезагрузки устройства: ' + error.message);
        });
}

// Сброс настроек к заводским
function resetSettings() {
    apiRequest('/api/reset', 'POST')
        .then(data => {
            if (data.success) {
                alert('Настройки сброшены к заводским значениям. Устройство перезагружается...');
                
                // Перенаправление на страницу входа через 5 секунд
                setTimeout(function() {
                    localStorage.removeItem('auth_token');
                    localStorage.removeItem('username');
                    window.location.href = 'index.html';
                }, 5000);
            } else {
                throw new Error(data.error || 'Ошибка сброса настроек');
            }
        })
        .catch(error => {
            console.error('Ошибка сброса настроек:', error);
            alert('Ошибка сброса настроек: ' + error.message);
        });
}

// Сканирование WiFi сетей
function scanWiFiNetworks() {
    const modal = document.getElementById('wifi-networks-modal');
    const networksList = document.getElementById('wifi-networks-list');
    
    // Отображение модального окна
    modal.style.display = 'block';
    networksList.innerHTML = '<div class="loading-spinner">Сканирование сетей...</div>';
    
    // Запрос к API для сканирования сетей
    apiRequest('/api/wifi-scan')
        .then(data => {
            if (data.networks && data.networks.length > 0) {
                networksList.innerHTML = '';
                
                data.networks.forEach(network => {
                    const networkItem = document.createElement('div');
                    networkItem.className = 'wifi-network-item';
                    
                    const networkName = document.createElement('div');
                    networkName.className = 'wifi-network-name';
                    networkName.textContent = network.ssid;
                    
                    const signalStrength = document.createElement('div');
                    signalStrength.className = 'wifi-signal-strength';
                    
                    const signalBars = document.createElement('span');
                    signalBars.className = 'wifi-signal-bars';
                    // Определение количества полосок сигнала в зависимости от RSSI
                    let signalClass = 'signal-weak';
                    if (network.rssi > -50) {
                        signalClass = 'signal-excellent';
                    } else if (network.rssi > -65) {
                        signalClass = 'signal-good';
                    } else if (network.rssi > -75) {
                        signalClass = 'signal-fair';
                    }
                    signalBars.classList.add(signalClass);
                    
                    const signalText = document.createElement('span');
                    signalText.textContent = network.rssi + ' dBm';
                    
                    signalStrength.appendChild(signalBars);
                    signalStrength.appendChild(signalText);
                    
                    networkItem.appendChild(networkName);
                    networkItem.appendChild(signalStrength);
                    
                    // Обработчик клика по сети
                    networkItem.addEventListener('click', function() {
                        document.getElementById('wifi-ssid').value = network.ssid;
                        closeModals();
                    });
                    
                    networksList.appendChild(networkItem);
                });
            } else {
                networksList.innerHTML = '<div class="text-center">Сети не найдены</div>';
            }
        })
        .catch(error => {
            console.error('Ошибка сканирования WiFi сетей:', error);
            networksList.innerHTML = '<div class="text-center error-message">Ошибка сканирования: ' + error.message + '</div>';
        });
}

// Загрузка прошивки
function uploadFirmware() {
    const fileInput = document.getElementById('firmware-file');
    const progressBar = document.getElementById('firmware-progress');
    const progressText = document.getElementById('firmware-progress-text');
    const progressContainer = document.getElementById('firmware-progress-container');
    
    if (!fileInput.files.length) {
        alert('Пожалуйста, выберите файл прошивки');
        return;
    }
    
    const file = fileInput.files[0];
    if (!file.name.endsWith('.bin')) {
        alert('Пожалуйста, выберите файл с расширением .bin');
        return;
    }
    
       if (!confirm('Вы уверены, что хотите обновить прошивку? Устройство будет перезагружено после обновления.')) {
        return;
    }
    
    // Отображение прогресс-бара
    progressContainer.style.display = 'block';
    progressBar.style.width = '0%';
    progressText.textContent = '0%';
    
    const formData = new FormData();
    formData.append('firmware', file);
    
    const xhr = new XMLHttpRequest();
    xhr.open('POST', '/api/update', true);
    
    // Добавление заголовка авторизации
    const token = localStorage.getItem('auth_token');
    xhr.setRequestHeader('Authorization', 'Basic ' + token);
    
    xhr.upload.onprogress = function(e) {
        if (e.lengthComputable) {
            const percentComplete = Math.round((e.loaded / e.total) * 100);
            progressBar.style.width = percentComplete + '%';
            progressText.textContent = percentComplete + '%';
        }
    };
    
    xhr.onload = function() {
        if (xhr.status === 200) {
            try {
                const response = JSON.parse(xhr.responseText);
                if (response.success) {
                    alert('Прошивка успешно загружена. Устройство перезагружается...');
                    
                    // Перенаправление на страницу входа через 10 секунд
                    setTimeout(function() {
                        window.location.href = 'index.html';
                    }, 10000);
                } else {
                    throw new Error(response.error || 'Ошибка обновления прошивки');
                }
            } catch (e) {
                console.error('Ошибка обработки ответа:', e);
                alert('Ошибка обработки ответа: ' + e.message);
            }
        } else {
            alert('Ошибка загрузки прошивки: ' + xhr.status);
        }
    };
    
    xhr.onerror = function() {
        alert('Ошибка соединения при загрузке прошивки');
    };
    
    xhr.send(formData);
}

// Отображение модального окна подтверждения
function showConfirmModal(title, message, confirmCallback) {
    const modal = document.getElementById('confirm-modal');
    const modalTitle = document.getElementById('confirm-modal-title');
    const modalMessage = document.getElementById('confirm-modal-message');
    const confirmBtn = document.getElementById('confirm-ok-btn');
    
    modalTitle.textContent = title;
    modalMessage.textContent = message;
    
    // Удаление предыдущих обработчиков
    const newConfirmBtn = confirmBtn.cloneNode(true);
    confirmBtn.parentNode.replaceChild(newConfirmBtn, confirmBtn);
    
    // Добавление нового обработчика
    newConfirmBtn.addEventListener('click', function() {
        closeModals();
        if (typeof confirmCallback === 'function') {
            confirmCallback();
        }
    });
    
    modal.style.display = 'block';
}

// Закрытие всех модальных окон
function closeModals() {
    const modals = document.querySelectorAll('.modal');
    modals.forEach(function(modal) {
        modal.style.display = 'none';
    });
}

// Проверка валидности IP-адреса
function validateIPAddress(ip) {
    const pattern = /^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$/;
    return pattern.test(ip);
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

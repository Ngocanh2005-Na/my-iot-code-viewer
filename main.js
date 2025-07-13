
// LƯU Ý: Thay thế các giá trị này bằng thông tin Firebase của bạn!
const firebaseConfig = {
    apiKey: "AIzaSyAmerwMllKGTowx3aMVfIUlr7aeYCiNBWs", // API Key 
    databaseURL: "https://final-project-esp32-e3bdf-default-rtdb.asia-southeast1.firebasedatabase.app/", // Database URL 
};

// Khởi tạo Firebase
firebase.initializeApp(firebaseConfig);
const database = firebase.database();

// Tham chiếu đến các node trong Realtime Database
const sensorRef = database.ref('sensor');
const deviceRef = database.ref('device');

// ===============================================
// Lắng nghe dữ liệu cảm biến (Sensor Data)
// ===============================================
sensorRef.on('value', (snapshot) => {
    const data = snapshot.val();
    if (data) {
        document.getElementById('temperature').textContent = data.temperature ? data.temperature.toFixed(1) : '--';
        document.getElementById('humidity').textContent = data.humidity ? Math.round(data.humidity) : '--';
        document.getElementById('lightLevel').textContent = data.light ? data.light : '--';
        document.getElementById('soilMoisture').textContent = data.soil ? data.soil : '--';
        document.getElementById('gasLevel').textContent = data.gas ? data.gas.toFixed(1) : '--';
    }
});

// ===============================================
// Lắng nghe trạng thái thiết bị (Device Status)
// ===============================================
deviceRef.on('value', (snapshot) => {
    const data = snapshot.val();
    if (data) {
        // Cập nhật trạng thái LED
        const ledToggle = document.getElementById('ledToggle');
        const ledStatusDot = document.getElementById('ledStatusDot');
        if (data.led !== undefined) {
            ledToggle.checked = data.led; // Cập nhật trạng thái của switch
            if (data.led) {
                ledStatusDot.classList.add('active'); // Thêm class active để đổi màu dot
            } else {
                ledStatusDot.classList.remove('active');
            }
        }

        // Cập nhật trạng thái Pump
        const pumpToggle = document.getElementById('pumpToggle');
        const pumpStatusDot = document.getElementById('pumpStatusDot');
        if (data.pump !== undefined) {
            pumpToggle.checked = data.pump; // Cập nhật trạng thái của switch
            if (data.pump) {
                pumpStatusDot.classList.add('active'); // Thêm class active để đổi màu dot
            } else {
                pumpStatusDot.classList.remove('active');
            }
        }
    }
});

// ===============================================
// Hàm gửi lệnh điều khiển lên Firebase khi toggle switch thay đổi
// ===============================================
document.getElementById('ledToggle').addEventListener('change', (event) => {
    const state = event.target.checked;
    deviceRef.child('led').set(state)
        .then(() => console.log(`LED set to ${state}`))
        .catch(error => console.error("Error setting LED state: ", error));
});

document.getElementById('pumpToggle').addEventListener('change', (event) => {
    const state = event.target.checked;
    deviceRef.child('pump').set(state)
        .then(() => console.log(`Pump set to ${state}`))
        .catch(error => console.error("Error setting Pump state: ", error));
});
const express = require('express');
const { exec } = require('child_process');

const app = express();
app.use(express.static('public')); // public 폴더를 정적 파일 경로로 설정

app.get('/pointcloud', (req, res) => {
    exec('../build/Release/Youth', (error, stdout, stderr) => {
        if (error) {
            console.error(`Error executing C++ code: ${error.message}`);
            return res.status(500).send('Error executing C++ code');
        }
        if (stderr) {
            console.error(`stderr: ${stderr}`);
            return res.status(500).send('Error in C++ code');
        }
        
        // JSON 데이터 출력
        console.log('JSON Data:', stdout);
        
        try {
            const pointCloudData = JSON.parse(stdout);
            res.json(pointCloudData);
        } catch (parseError) {
            console.error('Failed to parse JSON:', parseError);
            res.status(500).send('Failed to parse JSON');
        }
    });
});

app.listen(3000, () => {
    console.log('Server is running on http://localhost:3000');
});

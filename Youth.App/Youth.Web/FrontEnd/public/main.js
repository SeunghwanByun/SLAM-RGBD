async function fetchPointCloudData() {
    console.log('Requesting point cloud data...');
    const response = await fetch('/pointcloud');
    const data = await response.json();
    
    // 받은 데이터 출력
    console.log('Received Point Cloud Data:', data);
    
    return data;
}

const scene = new THREE.Scene();
const camera = new THREE.PerspectiveCamera(75, window.innerWidth / window.innerHeight, 0.1, 1000);
const renderer = new THREE.WebGLRenderer();
renderer.setSize(window.innerWidth, window.innerHeight);
document.body.appendChild(renderer.domElement);

camera.position.z = 5;

async function createPointCloud() {
    const data = await fetchPointCloudData();

    if (!data || data.length === 0) {
        console.error("No data received from server");
        return;
    }

    console.log("Point Cloud Data: ", data);  // 데이터를 확인하기 위한 로그

    const geometry = new THREE.BufferGeometry();
    const positions = new Float32Array(data.length * 3);

    for (let i = 0; i < data.length; i++) {
        const point = data[i];
        positions[i * 3] = point.x;
        positions[i * 3 + 1] = point.y;
        positions[i * 3 + 2] = point.z;
    }

    geometry.setAttribute('position', new THREE.BufferAttribute(positions, 3));

    const material = new THREE.PointsMaterial({ color: 0xffffff, size: 0.05 });
    const pointCloud = new THREE.Points(geometry, material);

    scene.add(pointCloud);
}

createPointCloud();

function animate() {
    requestAnimationFrame(animate);
    renderer.render(scene, camera);
}

animate();

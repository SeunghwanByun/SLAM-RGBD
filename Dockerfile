# 1. Ubuntu 24.04 이미지를 베이스로 설정
FROM ubuntu:24.04

# 2. 필수 패키지 설치 (C 컴파일러 및 기타 개발 도구)
RUN apt update && apt install -y build-essential

# 3. 작업 디렉토리 설정
WORKDIR /Project/Astra-Drone-SLAM

# 4. 소스 코드 복사
COPY ./ /Project/Astra-Drone-SLAM/

# 5. 소스 코드 컴파일
RUN cmake -S. -Bbuild/Release -DCMAKE_BUILD_TYPE=Release

# 6. 프로그램 실행 명령어 설정
CMD ./Release/Youth
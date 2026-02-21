import cv2
import numpy as np

cap = cv2.VideoCapture(0)

if not cap.isOpened():
    print("Camera could not be opened!")
    exit()

while True:
    ret, frame = cap.read()

    if not ret:
        exit()
    
    frame = cv2.resize(frame, (640, 480))

    blurred = cv2.GaussianBlur(frame,(7, 7), 0)

    hsv = cv2.cvtColor(blurred, cv2.COLOR_BGR2HSV)


    lower = np.array([35, 50, 50]) 
    upper = np.array([85, 255, 255])

    mask =cv2.inRange(hsv, lower, upper)

    kernel = np.ones((3, 3), np.uint8)  #examines pixels, larger the matrix more aggresive cleaning 
    mask = cv2.morphologyEx(mask, cv2.MORPH_OPEN, kernel)   #deletes little white dots and enlarges white dots
    mask = cv2.morphologyEx(mask, cv2.MORPH_CLOSE, kernel)  #deletes little black dots inside the larger white dots

    result = cv2.bitwise_and(frame, frame, mask=mask)

    cv2.imshow("Original", frame)
    cv2.imshow("Blurred", blurred)
    cv2.imshow("HSV", hsv)
    cv2.imshow("Masked", mask)
    cv2.imshow("Result", result)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
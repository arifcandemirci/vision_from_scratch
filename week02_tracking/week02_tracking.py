"""
This script performs real-time color-based object tracking using OpenCV.
It segments a target color in HSV space, cleans the mask using morphological operations,
and tracks the most suitable contour while applying exponential moving average (EMA) filter for stable center estimation.
"""

import cv2
import numpy as np
import time

class ColourContourTracker:

    def __init__(self,
                 hsv_lower= (35, 50, 50),
                 hsv_upper=(85, 255, 255),
                 kernel_size=3,
                 area_min=500,
                 alpha=0.55,
                 lost_max=10,
            ):
        
        #---Parameters---
        self.hsv_lower = np.array(hsv_lower, dtype=np.uint8)
        self.hsv_upper = np.array(hsv_upper, dtype=np.uint8)
        self.kernel_size = kernel_size
        self.area_min = area_min
        self.alpha = alpha
        self.lost_max = lost_max

        #State
        self.cx_f = None
        self.cy_f = None
        self.lost_count = 0

    def segment(self, frame_bgr) -> np.ndarray:
        """makes it blurred then converts BGR to HSV, threshold by range,
        clean with morphology output is a binary mask"""

        blurred = cv2.GaussianBlur(frame_bgr, (7, 7), 0)
        hsv = cv2.cvtColor(blurred, cv2.COLOR_BGR2HSV)

        mask = cv2.inRange(hsv, self.hsv_lower, self.hsv_upper)

        k = max(1, int(self.kernel_size))   #kernel size must be a odd number
        if k % 2 == 0:
            k +=1
        kernel = np.ones((k, k), np.uint8) #create a matrix that size is k x k

        mask = cv2.morphologyEx(mask, cv2.MORPH_OPEN, kernel)   #deletes little white dots and enlarges white dots
        mask = cv2.morphologyEx(mask, cv2.MORPH_CLOSE, kernel)  #deletes little black dots inside the larger white dots

        return mask
    
    def select_best_contour(self, mask):
        """
        chooses the best contour using a score
        score = area * circularity
        """

        contours, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

        best = None
        best_score = -1.0

        for c in contours:
            area = cv2.contourArea(c)
            if area < self.area_min :
                continue

            peri = cv2.arcLength(c, True)
            if peri <= 0:
                continue

            circularity = 4.0 * np.pi * area / (peri * peri)
            score= area * circularity

            if score > best_score:
                best_score = score
                best = c
        
        return best
            
    def update(self, contour):
        """
        updates internal tracking state with EMA and lost handling
        returns tracking info
        """

        info={
            "found": False,
            "cx": None,
            "cy": None,
            "cx_f": self.cx_f,
            "cy_f": self.cy_f,
            "bbox": None,
            "area": None,
            "lost_count": self.lost_count
        }
        #if there is no target
        if contour is None:
            self.lost_count += 1
            if self.lost_count > self.lost_max:
                self.cx_f, self.cy_f = None, None
            info["self.lost_count"], info["self.cx_f"], info["self.cy_f"] = self.lost_count, self.cx_f, self.cy_f
            return info
        
        #if there is a target
        self.lost_count = 0
        area =cv2.contourArea(contour)
        x, y, w, h = cv2.boundingRect(contour)
        cx = x + w // 2 #pixel coordinats must be an integer
        cy = y + h // 2 

        #EMA filter
        if self.cx_f is None:
            self.cx_f, self.cy_f = cx, cy
        else:
            a = self.alpha
            self.cx_f = int(a * cx + (1 - a) * self.cx_f)
            self.cy_f = int(a * cy + (1 - a) * self.cy_f)

        #update info
        info.update({
        "found": True,
        "cx": cx,
        "cy": cy,
        "cx_f": self.cx_f,
        "cy_f": self.cy_f,
        "bbox": (x, y, w, h),
        "area": area,
        "lost_count": self.lost_count
        })
        
        print([info])

        return info
    
    def draw(self, frame, info):
        if not info["found"]:
            return

        x, y, w, h = info["bbox"]
        cv2.rectangle(frame, (x, y), (x + w, y + h), (0, 255, 0), 2)

        # raw center (white)
        cv2.circle(frame, (info["cx"], info["cy"]), 4, (255, 255, 255), -1)

        # filtered center (red)
        cv2.circle(frame, (info["cx_f"], info["cy_f"]), 7, (0, 0, 255), -1)

    
def main():
            
    cap = cv2.VideoCapture(0)

    if not cap.isOpened():
        print("Camera could not be opened!")
        exit()

    tracker = ColourContourTracker()

    t_prev = time.time()
    fps = 0.0



    while True:
        ret, frame = cap.read()

        if not ret:
            exit()
        
        frame = cv2.resize(frame, (640, 480))

        #segmenting
        mask =tracker.segment(frame)

        #selecting target
        best = tracker.select_best_contour(mask)

        #updating the tracking state
        info = tracker.update(best)

        #fps
        t_now = time.time()
        dt = t_now - t_prev
        t_prev = t_now
        if dt > 0:
            fps = 1.0 / dt

        status = "FOUND" if info["found"] else f"LOST({info['lost_count']})"

        cv2.putText(frame, f"{status} FPS:{fps:.1f}", (10, 25), cv2.FONT_HERSHEY_COMPLEX, 0.6, (255, 255, 255), 2)

        tracker.draw(frame, info)

        result = cv2.bitwise_and(frame,frame, mask=mask)

        cv2.imshow("Original", frame)
        cv2.imshow("Masked", mask)
        cv2.imshow("Result", result)

        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    cap.release()
    cv2.destroyAllWindows()

if __name__ == "__main__":
    main()
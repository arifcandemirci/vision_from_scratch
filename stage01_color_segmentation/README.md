# Week 1: Color Based Segmentation 🎨

Bu modül, bir görüntü akışından belirli bir rengin (örneğin yeşil) ayrıştırılması ve elde edilen binary (ikili) maskenin görüntü işleme teknikleriyle iyileştirilmesi üzerine odaklanmaktadır.

## 🧠 Teknik Kavramlar

### 1. HSV (Hue, Saturation, Value) Renk Uzayı
Geleneksel BGR (Blue-Green-Red) yerine HSV kullanmamızın temel nedeni, **aydınlatma varyasyonlarına karşı dirençtir**. 
-   **Hue (Ton)**: Rengin türünü belirtir (0-179).
-   **Saturation (Doygunluk)**: Rengin canlılığını belirtir (0-255).
-   **Value (Parlaklık)**: Rengin yoğunluğunu/parlaklığını belirtir (0-255).

### 2. Morfolojik Filtreleme
Segmentasyon sonrası oluşan maske genellikle küçük pikseller ("tuz ve biber" gürültüsü) içerir. Bunları temizlemek için matematiksel morfoloji kullanılır:

-   **Opening (Açma)**: `Erosion` (Erozyon) sonrası `Dilation` (Genişleme). Küçük beyaz gürültüleri yok eder.
-   **Closing (Kapama)**: `Dilation` sonrası `Erosion`. Nesne içindeki küçük siyah delikleri doldurur.

## 🛠️ Uygulama Adımları
1.  **Preprocessing**: Görüntü `GaussianBlur` (7x7) ile yumuşatılarak sensör gürültüsü azaltılır.
2.  **Conversion**: `cv2.cvtColor` ile BGR verisi HSV uzayına taşınır.
3.  **Thresholding**: `cv2.inRange` fonksiyonu ile hedeflenen yeşil tonları izole edilir.
4.  **Refinement**: 3x3 kernel kullanılarak Opening ve Closing işlemleri uygulanır.
5.  **Masking**: Bitwise-AND operatörü ile orijinal görüntü üzerinden sadece maskelenen bölge gösterilir.

## 🏃 Çalıştırma
```bash
python week01_color_segmentation/week01_color_segmentation.py
```
*Çıkış için 'q' tuşunu kullanınız.*

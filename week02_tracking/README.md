# Week 2: Real-Time Object Tracking 🎯

Bu modülde, renk tabanlı segmentasyon verileri kullanılarak dinamik bir nesne takip sistemi (Tracker) inşa edilmiştir. Proje, bir **yeşil şişe kapağını** gürültülü ortamlarda bile kararlı bir şekilde takip etmek üzere tasarlanmıştır.

## 🚀 Case Study: Yeşil Şişe Kapağı Takibi

Şişe kapağı gibi dairesel ve belirli bir renge sahip nesneleri takip ederken karşılaşılan zorluklar ve uygulanan çözümler:

### 1. Akıllı Kontur Seçimi (Circularity Score)
Görüntüdeki her yeşil nesne hedefimiz olmayabilir. Gerçek hedefi bulmak için sadece alana (`area`) değil, nesnenin ne kadar "yuvarlak" olduğuna da bakarız:
$$Circularity = \frac{4 \pi \times Area}{Perimeter^2}$$
*Kusursuz bir dairenin skoru 1.0'dır.* Sistem, alanı yeterli büyüklükte olan ve dairesellik skoru en yüksek olan konturu seçer.

### 2. EMA (Exponential Moving Average) Filtresi
Kamera titremeleri veya anlık ışık değişimleri, nesne merkezinin (centroid) titremesine neden olur. Bu titreşimi sönümlemek için EMA filtresi uygulanır:
$$x_{filtered} = \alpha \cdot x_{raw} + (1 - \alpha) \cdot x_{filtered\_prev}$$
-   **$\alpha$ (Smoothing Factor)**: Genellikle 0.55 olarak ayarlanmıştır. Takip hızını ve yumuşaklığını dengeler.

### 3. State-Machine & Lost Handling
Nesne geçici olarak kapandığında veya kameradan çıktığında sistem anında "kayboldu" demez. `lost_max` parametresi kadar bekler, bu süreçte son bilinen konumunu hatırlar.

## 🛠️ Teknik Özellikler
-   **Class `ColourContourTracker`**: Modüler ve tekrar kullanılabilir takip merkezi.
-   **Real-time FPS**: Verimlilik odaklı algoritma tasarımı ile yüksek kare hızı.
-   **Visual Feedback**:
    -   **Yeşil Bounding Box**: Hedef nesnenin kapladığı alan.
    -   **Beyaz Nokta**: Ham veri merkezi.
    -   **Kırmızı Nokta**: Filtrelenmiş, kararlı takip merkezi.

## 🏃 Çalıştırma
```bash
python week02_tracking/week02_tracking.py
```
*Not: En iyi sonuç için iyi aydınlatılmış bir ortamda yeşil bir şişe kapağı kullanınız.*

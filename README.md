🌪️ WindOS - Bağımsız İşletim Sistemi / Independent Operating System
(For English, please scroll down) ⬇️

🇹🇷 [TÜRKÇE]
WindOS, x86 (32-bit) mimarisi üzerinde hiçbir dış kütüphane (libc) kullanılmadan, sıfırdan saf C ve Assembly ile geliştirilmiş bağımsız bir işletim sistemidir.

Sistem, sıradan bir hobi projesi olmaktan çıkıp; kendi dosya sistemine, evrensel paket yükleyicisine ve donanım düzeyinde ekran optimizasyonlarına sahip tam teşekküllü bir ekosisteme dönüşmüştür.

👤 Lead Developer / System Architect: Efe (Efe-Elite-Dev)

✨ Öne Çıkan Dev Özellikler
📦 Evrensel Paket Yükleyici (Universal Installer): WindOS'un kendine ait resmi .wpk formatının yanı sıra, .exe, .apk ve .deb uzantılarını tanıyıp simüle edebilen çoklu kurulum motoru.

🪟 Aero Glass (Şeffaf Mod): İşlemciyi yormadan çalışan alfa harmanlamalı (Alpha Blending) şeffaf pencere motoru. (Klavye kısayolu: T)

🔄 4D Ekran Düzeltici (Donanım Senkronizatörü): QEMU ve VirtualBox'ın FrameBuffer ayna/ters dönme hatalarını donanım seviyesinde sıfır kasmayla düzelten algoritma. (Klavye kısayolu: F)

🧠 WindAI Entegrasyonu: Sistem geneline entegre edilmiş, yerel (offline) yapay zeka asistanı arayüzü. (Klavye kısayolu: Alt + A)

💾 Dinamik Donanım Analizi: Multiboot üzerinden fiziksel RAM'i ve depolama limitlerini anlık okuma yeteneği.

🌐 CloudBrowser & WindNot: Kendi wpk:// ağ protokolüyle çalışan bulut tarayıcı ve gömülü metin düzenleyici.

🚀 Kurulum ve Derleme
WindOS, Freestanding ortamında GCC ile derlenmek üzere tasarlanmıştır.

Bash
gcc -m32 -ffreestanding -fno-builtin -fno-stack-protector -O2 -Wall -c kernel.c -o kernel.o
ld -m elf_i386 -T linker.ld boot.o kernel.o -o kernel.bin
Not: En iyi görüntü deneyimi için QEMU kullanılması tavsiye edilir. Görüntüde terslik yaşarsanız sistem içindeyken F tuşuna basarak ekranı senkronize edebilirsiniz.

🗺️ Yol Haritası (Roadmap)
[x] V13 Stable (x86 PC): Çekirdeğin ana modüllerinin, UI tasarımının ve evrensel yükleyicinin hatasız çalışması.

[ ] ARM Mimarisine Geçiş: C kodlarının akıllı telefonlar için uygun hale getirilmesi.

[ ] Samsung Galaxy A04s (Exynos) Portu: WindOS'un mobil cihazlarda çalışacak ilk fiziksel testi (XDA Developers üzerinden süreç paylaşılacaktır).

⚠️ Sorumluluk Reddi (Disclaimer): WindOS tamamen deneysel ve eğitim amaçlı geliştirilmiş bir işletim sistemidir. Bu sistemin derlenmesi, çalıştırılması veya fiziksel/sanal cihazlara kurulması sırasında oluşabilecek yazılımsal/donanımsal hatalardan, sistem çökmelerinden veya veri kayıplarından geliştirici sorumlu tutulamaz. Kullanım riski tamamen kullanıcıya aittir.

🇬🇧 [ENGLISH]
WindOS is an independent operating system developed from scratch in pure C and Assembly on the x86 (32-bit) architecture, entirely without the use of external libraries (libc).

It has evolved from a simple hobby project into a fully-fledged ecosystem featuring its own file system, universal package installer, and hardware-level screen optimizations.

👤 Lead Developer / System Architect: Efe (Efe-Elite-Dev)

✨ Core Features
📦 Universal Installer: A multi-installation engine capable of simulating and recognizing .exe, .apk, and .deb extensions, alongside WindOS's official .wpk format.

🪟 Aero Glass (Transparent Mode): A highly optimized alpha-blending transparent window engine. (Hotkey: T)

🔄 4D Screen Fixer (Hardware Synchronizer): A zero-lag hardware-level algorithm that fixes FrameBuffer mirroring/flipping issues common in QEMU and VirtualBox. (Hotkey: F)

🧠 WindAI Integration: A system-wide, locally simulated offline AI assistant interface. (Hotkey: Alt + A)

💾 Dynamic Hardware Analysis: Real-time physical RAM and storage limit detection via Multiboot.

🌐 CloudBrowser & WindNot: A cloud browser operating on its own wpk:// network protocol and a built-in text editor.

🚀 Build Instructions
WindOS is designed to be compiled with GCC in a freestanding environment.

Bash
gcc -m32 -ffreestanding -fno-builtin -fno-stack-protector -O2 -Wall -c kernel.c -o kernel.o
ld -m elf_i386 -T linker.ld boot.o kernel.o -o kernel.bin
Note: QEMU is highly recommended for the best visual experience. If you experience screen mirroring/flipping, simply press the F key while the OS is running to synchronize the display.

🗺️ Roadmap
[x] V13 Stable (x86 PC): Flawless execution of core kernel modules, UI design, and universal installer.


[ ] ARM Architecture Transition: Refactoring C code for smartphone compatibility.

[ ] Samsung Galaxy A04s (Exynos) Port: The first physical hardware test of WindOS on mobile devices (Development process will be shared on XDA Developers).

⚠️ Disclaimer: WindOS is an experimental operating system developed purely for educational and research purposes. The developer is not responsible for any software/hardware bugs, system crashes, or data loss that may occur while compiling, running, or installing this system on physical or virtual machines. Use at your own risk.

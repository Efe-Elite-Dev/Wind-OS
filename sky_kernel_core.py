import customtkinter as ctk
from sky_setup_ui import SkyCoreSetup

class SkyKernelCore(ctk.CTk):
    def __init__(self):
        super().__init__()
        
        self.title("Sky Core OS v1.5 [vortex-kernel] - Active")
        self.geometry("1024x768")
        self.resizable(False, False)
        
        # Ana Kernel Arka Planı
        self.kernel_frame = ctk.CTkFrame(self, fg_color="#0f0a1c")
        self.kernel_frame.pack(fill="both", expand=True)
        
        self.load_main_os_interface()

    def load_main_os_interface(self):
        status_lbl = ctk.CTkLabel(
            self.kernel_frame, 
            text="🌪️ SKY CORE OS [ vortex-kernel v1.5 ] AKTİF\nSistem başarıyla ana belleğe yüklendi ve çalışıyor.", 
            font=("Arial", 22, "bold"), 
            text_color="#00d2d3"
        )
        status_lbl.pack(pady=60)
        
        # Burası senin asıl işletim sisteminin terminal simülasyonudur
        terminal_view = ctk.CTkTextbox(self.kernel_frame, width=850, height=450, fg_color="black", text_color="#1dd1a1", font=("Consolas", 13))
        terminal_view.pack(pady=10)
        terminal_view.insert("0.0", "sky_core@vortex-kernel:~$ core_init --status SUCCESS\nsky_core@vortex-kernel:~$ Sihirbaz tamamlandı, arayüz belleği serbest bırakıldı.\nsky_core@vortex-kernel:~$ kernel_status: ONLINE\nsky_core@vortex-kernel:~$ ")

def run_main_system():
    # Sihirbaz tamamen yok edildikten sonra burası çalışır
    print("[BOOT] Kurulum arayüzünden başarıyla çıkıldı. Ana kernel yükleniyor...")
    main_os = SkyKernelCore()
    main_os.mainloop()

if __name__ == "__main__":
    print("[BOOT] Sky Core OS Kurulum Sihirbazı Çağrılıyor...")
    # Önce sihirbazı çalıştırıyoruz
    wizard = SkyCoreSetup()
    wizard.mainloop()
    
    # Sihirbazın mainloop'u içerideki quit() fonksiyonuyla kırıldığı an kod bu alt satıra akar:
    run_main_system()

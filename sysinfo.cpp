/*
 *@Author: Wanjare Samuel
 * @APPNAME: SysInfoViewer
 * PROGRAM: System Information Viewer
 * DESCRIPTION: View Information About your system with ease
 * LIBRARY: wxWidgets 3.2.5
 * FROM: 1st July 2024 Alego, Kenya.
 * BUILDING INFO: This app was compiled using -std=c++23 flag and g++ Version
 14,
 * Written in Zed textEditor Version 0.143
 *
 * WARN: This is a semi huge monolith tread carefully
 *
*/

// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

// #include <wx/button.h>
#include <string>
#include <vector>
#include <format>
#include <wx/app.h>
#include <wx/file.h>
#include <wx/filename.h>
#include <wx/icon.h>
#include <wx/listctrl.h>
#include <wx/menu.h>
#include <wx/platform.h>
#include <wx/platinfo.h>
#include <wx/process.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

#include <wx/treebook.h>
#include <wx/txtstrm.h>
#include <wx/utils.h>
#include <wx/version.h>
#include <wx/versioninfo.h>

// Drawing libraries
#include <cmath>
#include <sys/sysinfo.h>
#include <wx/graphics.h>

#include <chrono>
#include <cmath>
#include <fstream>
#include <sys/statvfs.h>
#include <thread>
#include <wx/dcbuffer.h>

#include <fstream>
#include <wx/dir.h>
#include <wx/log.h>
#include <wx/stc/stc.h>
#include <wx/textfile.h>
#include <wx/tokenzr.h>

// Network Inforamtion
#include <arpa/inet.h>
#include <array>
#include <atomic>
#include <chrono>
#include <cstring>
#include <curl/curl.h>
#include <errno.h>
#include <exception>
#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <ifaddrs.h>
#include <linux/if_link.h>
#include <linux/if_packet.h>
#include <map>
#include <memory>
#include <mutex>
#include <net/if.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <random>
#include <resolv.h>
#include <string>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <wx/graphics.h>
#include <wx/panel.h>
#include <wx/scrolwin.h>
#include <wx/sizer.h>
#include <wx/statbmp.h>
#include <wx/stattext.h>
#include <wx/thread.h>
#include <wx/event.h>

// storage
#include <sys/statvfs.h>
#include <wx/dir.h>
#include <wx/textfile.h>

// display libs
#include <wx/display.h>

// sound check
#include <alsa/asoundlib.h>

// app info
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <wx/listctrl.h>
#include <wx/wrapsizer.h>

#include <fstream>
#include <optional>
#include <regex>
#include <wx/font.h>


class MyApp : public wxApp {
public:
  virtual bool OnInit();
};

class MyFrame : public wxFrame {
public:
  MyFrame();

private:
  void OnHello(wxCommandEvent &event);
  void OnExit(wxCommandEvent &event);
  void OnAbout(wxCommandEvent &event);
};

enum { ID_Hello = 1 };

wxIMPLEMENT_APP(MyApp);

bool MyApp::OnInit() {
  wxLog::SetActiveTarget(new wxLogStderr());
  MyFrame *frame = new MyFrame();
  frame->SetSize(1200, 800);

  // Set App Icon!
  wxIcon icon(wxT("favicon.ico"), wxBITMAP_TYPE_ICO);
  SetAppDisplayName("System Info Viewer");
  wxInitAllImageHandlers();

#ifdef __WXMSW__
  SetIcon(icon);
#endif

  // create wxTreebook
  wxTreebook *treebook = new wxTreebook(frame, wxID_ANY, wxDefaultPosition,
                                        wxDefaultSize, wxNB_LEFT);

  // CREATE FOUR PANELS FOR THE PAGES
  wxPanel *systeminfoPage = new wxPanel(treebook, wxID_ANY);
  wxPanel *resourcesInfoPage = new wxPanel(treebook, wxID_ANY);
  wxPanel *miscInfoPage = new wxPanel(treebook, wxID_ANY);
  wxPanel *appsInfoPage = new wxPanel(treebook, wxID_ANY);

  /*********** BEGIN PAGE 1: Sytem Information ****************************/
  // Create sizers for the two rows: top & bottom rows
  wxBoxSizer *topRowSizer = new wxBoxSizer(wxHORIZONTAL);
  wxBoxSizer *bottomRowSizer = new wxBoxSizer(wxHORIZONTAL);

  // Create top row and bottom row child box sizers
  wxBoxSizer *topRowLeftChild = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer *topRowCenterChild = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer *topRowRightChild = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer *bottomRowLeftChild = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer *bottomRowRightChild = new wxBoxSizer(wxVERTICAL);

  /************* Continue: Populate page 1 values ***************/

  //.............................  TOP ROW CONTENT
  //.............................................//
  //.................................... CPU USAGE
  //...............................................//
  wxStaticText *cpuUsageTitle =
      new wxStaticText(systeminfoPage, wxID_ANY, "CPU USAGE", wxDefaultPosition,
                       wxDefaultSize, wxALIGN_CENTER_HORIZONTAL);
  topRowLeftChild->Add(cpuUsageTitle, 0, wxEXPAND | wxALL, 5);

  // topRowLeftChild->Add(new wxStaticText(systeminfoPage, wxID_ANY, "CPU
  // USAGE"),
  //                      0, wxALL | wxALIGN_CENTRE_HORIZONTAL, 5);
  class CPUDoughnutChartPanel : public wxPanel {
  public:
    CPUDoughnutChartPanel(wxWindow *parent) : wxPanel(parent), m_cpuUsage(0.0) {
      Bind(wxEVT_PAINT, &CPUDoughnutChartPanel::OnPaint, this);

      // Start a timer to update the chart every second
      m_timer.SetOwner(this);
      m_timer.Start(1000);
      Bind(wxEVT_TIMER, &CPUDoughnutChartPanel::OnTimer, this);

      // Get CPU info
      GetCPUInfo();
    }

  private:
    void OnPaint(wxPaintEvent &event) {
      wxPaintDC dc(this);
      wxGraphicsContext *gc = wxGraphicsContext::Create(dc);

      if (gc) {
        DrawDoughnutChart(gc);
        delete gc;
      }
    }

    void OnTimer(wxTimerEvent &event) {
      UpdateCPUUsage();
      Refresh();
    }

    void GetCPUInfo() {
      std::ifstream cpuinfo("/proc/cpuinfo");
      std::string line;
      int count = 0;
      double speed = 0.0;

      while (std::getline(cpuinfo, line)) {
        if (line.substr(0, 9) == "processor") {
          count++;
        } else if (line.substr(0, 7) == "cpu MHz") {
          size_t pos = line.find(':');
          if (pos != std::string::npos) {
            speed = std::stod(line.substr(pos + 1));
          }
        }
      }

      m_cpuCount = count;
      m_cpuSpeed = speed;
    }

    void UpdateCPUUsage() {
      std::ifstream stat("/proc/stat");
      std::string line;
      std::getline(stat, line);
      sscanf(line.c_str(), "cpu %llu %llu %llu %llu", &m_user, &m_nice,
             &m_system, &m_idle);

      unsigned long long totalUsage = m_user + m_nice + m_system;
      unsigned long long total = totalUsage + m_idle;

      if (m_prevTotal != 0 && m_prevTotalUsage != 0) {
        unsigned long long totalDiff = total - m_prevTotal;
        unsigned long long usageDiff = totalUsage - m_prevTotalUsage;
        m_cpuUsage = static_cast<double>(usageDiff) / totalDiff * 100.0;
      }

      m_prevTotal = total;
      m_prevTotalUsage = totalUsage;
    }

    void DrawDoughnutChart(wxGraphicsContext *gc) {
      wxSize size = GetClientSize();
      double width = size.GetWidth();
      double height = size.GetHeight();
      double outerRadius = std::min(width, height) * 0.4;
      double innerRadius = outerRadius * 0.75;
      double centerX = width / 2;
      double centerY = height / 2;

      double usedAngle = 2 * M_PI * m_cpuUsage / 100.0;

      // Draw used CPU (light pink)
      gc->SetBrush(wxBrush(wxColor(255, 182, 193))); // Light Pink
      gc->SetPen(wxPen(wxColor(255, 255, 255), 2));

      wxGraphicsPath usedPath = gc->CreatePath();
      usedPath.MoveToPoint(centerX + innerRadius, centerY);
      usedPath.AddArc(centerX, centerY, outerRadius, 0, usedAngle, true);
      usedPath.AddArc(centerX, centerY, innerRadius, usedAngle, 0, false);
      usedPath.CloseSubpath();

      gc->FillPath(usedPath);
      gc->StrokePath(usedPath);

      // Draw unused CPU (blue)
      gc->SetBrush(wxBrush(wxColor(135, 206, 235))); // Sky Blue

      wxGraphicsPath unusedPath = gc->CreatePath();
      unusedPath.MoveToPoint(centerX + innerRadius * cos(usedAngle),
                             centerY + innerRadius * sin(usedAngle));
      unusedPath.AddArc(centerX, centerY, outerRadius, usedAngle, 2 * M_PI,
                        true);
      unusedPath.AddArc(centerX, centerY, innerRadius, 2 * M_PI, usedAngle,
                        false);
      unusedPath.CloseSubpath();

      gc->FillPath(unusedPath);
      gc->StrokePath(unusedPath);

      // Add CPU usage in the center
      wxString usageText = wxString::Format("%.1f%%", m_cpuUsage);
      gc->SetFont(wxFont(20, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
                         wxFONTWEIGHT_BOLD),
                  wxColor(165, 42, 42));
      double textWidth, textHeight;
      gc->GetTextExtent(usageText, &textWidth, &textHeight);
      gc->DrawText(usageText, centerX - textWidth / 2,
                   centerY - textHeight / 2);

      // Add CPU info below the chart
      wxString cpuInfo = wxString::Format("CPU Count: %d | CPU Speed: %.2f MHz",
                                          m_cpuCount, m_cpuSpeed);
      gc->SetFont(wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
                         wxFONTWEIGHT_NORMAL),
                  wxColor(80, 80, 80));
      gc->GetTextExtent(cpuInfo, &textWidth, &textHeight);
      gc->DrawText(cpuInfo, centerX - textWidth / 2, height - textHeight - 10);
    }

    wxTimer m_timer;
    double m_cpuUsage;
    int m_cpuCount;
    double m_cpuSpeed;
    unsigned long long m_user, m_nice, m_system, m_idle;
    unsigned long long m_prevTotal = 0, m_prevTotalUsage = 0;
  };

  // Add CPU USAGE chart panel to the left sizer
  CPUDoughnutChartPanel *cpuChartPanel =
      new CPUDoughnutChartPanel(systeminfoPage);
  // CPU_USAGE_CHART_PANEL* cpuUsageChartPanel = new
  // CPU_USAGE_CHART_PANEL(systeminfoPage);
  topRowLeftChild->Add(cpuChartPanel, 1, wxEXPAND | wxALL, 5);

  //..................................... END OF CPU USAGE
  //.......................................//

  //.......................................  MEMORY USAGE
  //.............................................
  topRowCenterChild->Add(
      new wxStaticText(systeminfoPage, wxID_ANY, "MEMORY USAGE"), 0,
      wxALL | wxALIGN_CENTRE_HORIZONTAL, 5);

  class MemoryDoughnutChartPanel : public wxPanel {
  public:
    MemoryDoughnutChartPanel(wxPanel *parent) : wxPanel(parent, wxID_ANY) {
      Bind(wxEVT_PAINT, &MemoryDoughnutChartPanel::OnPaint, this);

      // Start a timer to update the chart every second
      m_timer.SetOwner(this);
      m_timer.Start(1000);
      Bind(wxEVT_TIMER, &MemoryDoughnutChartPanel::OnTimer, this);
    }

  private:
    void OnPaint(wxPaintEvent &event) {
      wxPaintDC dc(this);
      wxGraphicsContext *gc = wxGraphicsContext::Create(dc);

      if (gc) {
        DrawDoughnutChart(gc);
        delete gc;
      }
    }

    void OnTimer(wxTimerEvent &event) {
      Refresh(); // Trigger a repaint
    }

    void GetMemoryInfo(unsigned long long &totalMem,
                       unsigned long long &freeMem) {
      struct sysinfo memInfo;
      sysinfo(&memInfo);

      totalMem = memInfo.totalram;
      totalMem *= memInfo.mem_unit;

      freeMem = memInfo.freeram;
      freeMem *= memInfo.mem_unit;
    }

    void DrawDoughnutChart(wxGraphicsContext *gc) {
      unsigned long long totalMemory, freeMemory;
      GetMemoryInfo(totalMemory, freeMemory);
      unsigned long long usedMemory = totalMemory - freeMemory;

      wxSize size = GetClientSize();
      double width = size.GetWidth();
      double height = size.GetHeight();
      double outerRadius = std::min(width, height) * 0.4;
      double innerRadius = outerRadius * 0.6;
      double centerX = width / 2;
      double centerY = height / 2;

      double usedAngle = 2 * M_PI * usedMemory / totalMemory;

      // Draw used memory (brown)
      gc->SetBrush(wxBrush(wxColor(165, 42, 42))); // Brown
      gc->SetPen(wxPen(wxColor(255, 255, 255), 2));

      wxGraphicsPath usedPath = gc->CreatePath();
      usedPath.MoveToPoint(centerX + innerRadius, centerY);
      usedPath.AddArc(centerX, centerY, outerRadius, 0, usedAngle, true);
      usedPath.AddArc(centerX, centerY, innerRadius, usedAngle, 0, false);
      usedPath.CloseSubpath();

      gc->FillPath(usedPath);
      gc->StrokePath(usedPath);

      // Draw free memory (green)
      gc->SetBrush(wxBrush(wxColor(0, 128, 0))); // Green

      wxGraphicsPath freePath = gc->CreatePath();
      freePath.MoveToPoint(centerX + innerRadius * cos(usedAngle),
                           centerY + innerRadius * sin(usedAngle));
      freePath.AddArc(centerX, centerY, outerRadius, usedAngle, 2 * M_PI, true);
      freePath.AddArc(centerX, centerY, innerRadius, 2 * M_PI, usedAngle,
                      false);
      freePath.CloseSubpath();

      gc->FillPath(freePath);
      gc->StrokePath(freePath);

      // Draw center circle
      gc->SetBrush(wxBrush(wxColor(240, 240, 240)));
      gc->SetPen(wxPen(wxColor(200, 200, 200), 1));
      gc->DrawEllipse(centerX - innerRadius, centerY - innerRadius,
                      innerRadius * 2, innerRadius * 2);

      // Add memory info in the center
      wxString memoryInfo =
          wxString::Format("Total: %.2f GB\nUsed: %.2f GB\nFree: %.2f GB",
                           totalMemory / (1024.0 * 1024 * 1024),
                           usedMemory / (1024.0 * 1024 * 1024),
                           freeMemory / (1024.0 * 1024 * 1024));

      gc->SetFont(wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
                         wxFONTWEIGHT_BOLD),
                  wxColor(80, 80, 80));
      double textWidth, textHeight;
      gc->GetTextExtent(memoryInfo, &textWidth, &textHeight);
      gc->DrawText(memoryInfo, centerX - textWidth / 2,
                   centerY - textHeight / 2);

      // Add labels
      DrawLabel(gc, "Used", wxColor(165, 42, 42), centerX, centerY, outerRadius,
                usedAngle / 2);
      DrawLabel(gc, "Free", wxColor(0, 128, 0), centerX, centerY, outerRadius,
                usedAngle + (2 * M_PI - usedAngle) / 2);
    }

    void DrawLabel(wxGraphicsContext *gc, const wxString &label,
                   const wxColor &color, double centerX, double centerY,
                   double radius, double angle) {
      double labelX = centerX + radius * 1.1 * cos(angle);
      double labelY = centerY + radius * 1.1 * sin(angle);

      gc->SetFont(wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
                         wxFONTWEIGHT_BOLD),
                  color);
      double textWidth, textHeight;
      gc->GetTextExtent(label, &textWidth, &textHeight);
      gc->DrawText(label, labelX - textWidth / 2, labelY - textHeight / 2);
    }

    wxTimer m_timer;
  };
  // Add Memory chart panel to sizer
  MemoryDoughnutChartPanel *memoryChartPanel =
      new MemoryDoughnutChartPanel(systeminfoPage);
  topRowCenterChild->Add(memoryChartPanel, 1, wxEXPAND | wxALL, 5);

  //.................... DISK USAGE .......................
  topRowRightChild->Add(
      new wxStaticText(systeminfoPage, wxID_ANY, "DISK USAGE"), 0,
      wxALL | wxALIGN_CENTRE_HORIZONTAL, 5);

  class DiskUsagePieChart : public wxPanel {
  public:
    DiskUsagePieChart(wxWindow *parent) : wxPanel(parent) {
      SetBackgroundStyle(wxBG_STYLE_PAINT);
      Bind(wxEVT_PAINT, &DiskUsagePieChart::OnPaint, this);
      UpdateDiskUsage();

      // Start a timer to update the chart every 60 seconds
      m_timer.SetOwner(this);
      m_timer.Start(60000);
      Bind(wxEVT_TIMER, &DiskUsagePieChart::OnTimer, this);
    }

  private:
    void OnPaint(wxPaintEvent &event) {
      wxAutoBufferedPaintDC dc(this);
      dc.Clear();

      wxGraphicsContext *gc = wxGraphicsContext::Create(dc);
      if (gc) {
        DrawPieChart(gc);
        delete gc;
      }
    }

    void OnTimer(wxTimerEvent &event) {
      UpdateDiskUsage();
      Refresh();
    }

    void UpdateDiskUsage() {
      struct statvfs stats;
      if (statvfs("/", &stats) == 0) {
        m_totalSpace = stats.f_blocks * stats.f_frsize;
        m_freeSpace = stats.f_bfree * stats.f_frsize;
        m_usedSpace = m_totalSpace - m_freeSpace;
      }
    }

    void DrawPieChart(wxGraphicsContext *gc) {
      wxSize size = GetClientSize();
      double width = size.GetWidth();
      double height = size.GetHeight();
      double radius = std::min(width, height) * 0.4;
      double centerX = width / 2;
      double centerY = height / 2;

      double usedAngle = 2 * M_PI * m_usedSpace / m_totalSpace;

      // Draw used space (pink)
      gc->SetBrush(wxBrush(wxColor(255, 182, 193))); // Light Pink
      gc->SetPen(wxPen(wxColor(255, 255, 255), 2));

      wxGraphicsPath usedPath = gc->CreatePath();
      usedPath.AddArc(centerX, centerY, radius, 0, usedAngle, true);
      usedPath.AddLineToPoint(centerX, centerY);
      usedPath.CloseSubpath();

      gc->FillPath(usedPath);
      gc->StrokePath(usedPath);

      // Draw free space (purple)
      gc->SetBrush(wxBrush(wxColor(230, 230, 250))); // Lavender

      wxGraphicsPath freePath = gc->CreatePath();
      freePath.AddArc(centerX, centerY, radius, usedAngle, 2 * M_PI, true);
      freePath.AddLineToPoint(centerX, centerY);
      freePath.CloseSubpath();

      gc->FillPath(freePath);
      gc->StrokePath(freePath);

      // Add labels
      gc->SetFont(wxFont(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
                         wxFONTWEIGHT_BOLD),
                  wxColor(164, 42, 42));

      DrawLabel(gc, "Total: " + FormatSize(m_totalSpace), centerX, height - 40);
      DrawLabel(gc, "Used: " + FormatSize(m_usedSpace), centerX - radius,
                height - 40);
      DrawLabel(gc, "Free: " + FormatSize(m_freeSpace), centerX + radius,
                height - 40);

      // Add percentage in the center
      wxString percentageText =
          wxString::Format("%.1f%%", (double)m_usedSpace / m_totalSpace * 100);
      gc->SetFont(wxFont(20, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
                         wxFONTWEIGHT_BOLD),
                  wxColor(80, 80, 80));
      double textWidth, textHeight;
      gc->GetTextExtent(percentageText, &textWidth, &textHeight);
      gc->DrawText(percentageText, centerX - textWidth / 2,
                   centerY - textHeight / 2);
    }

    void DrawLabel(wxGraphicsContext *gc, const wxString &text, double x,
                   double y) {
      double textWidth, textHeight;
      gc->GetTextExtent(text, &textWidth, &textHeight);
      gc->DrawText(text, x - textWidth / 2, y);
    }

    wxString FormatSize(uint64_t size) {
      const char *units[] = {"B", "KB", "MB", "GB", "TB"};
      int unitIndex = 0;
      double formattedSize = size;

      while (formattedSize >= 1024 && unitIndex < 4) {
        formattedSize /= 1024;
        unitIndex++;
      }

      return wxString::Format("%.2f %s", formattedSize, units[unitIndex]);
    }

    wxTimer m_timer;
    uint64_t m_totalSpace;
    uint64_t m_usedSpace;
    uint64_t m_freeSpace;
  };

  // Add Disk Usage chart panel to sizer
  DiskUsagePieChart *diskUsageChartPanel =
      new DiskUsagePieChart(systeminfoPage);
  topRowRightChild->Add(diskUsageChartPanel, 1, wxEXPAND | wxALL, 5);

  //............................ END OF TOP ROW CONTENT
  //...........................................//

  // ............................ OS INFORMATION CONTENT
  // ...........................................// START OF BOTTOM LEFT CONTENT
  // //
  bottomRowLeftChild->Add(
      new wxStaticText(systeminfoPage, wxID_ANY, "OS INFORMATION"), 0, wxALL,
      5);

  wxPlatformInfo platform;

  auto osCurrentTime = wxNow();
  wxString osDescription = platform.GetOperatingSystemDescription();
  wxString distroInfo = platform.GetLinuxDistributionInfo().Description;
  wxString userName = wxGetUserName();
  wxString de = platform.GetDesktopEnvironment();

#if defined(__linux__)
  wxLinuxDistributionInfo osDistroInfo = platform.GetLinuxDistributionInfo();
  wxStaticText *osDistCodeName =
      new wxStaticText(systeminfoPage, wxID_ANY, osDistroInfo.CodeName);

  bottomRowLeftChild->Add(osDistCodeName, 0, wxALL | wxEXPAND, 5);
#endif

  // Populate the wxBoxSizer
  wxStaticText *osUserNameCtrl =
      new wxStaticText(systeminfoPage, wxID_ANY, userName);

  wxStaticText *osNameCtrl =
      new wxStaticText(systeminfoPage, wxID_ANY, osCurrentTime);
  wxStaticText *osDescCtrl =
      new wxStaticText(systeminfoPage, wxID_ANY, osDescription);
  wxStaticText *osDistroCtrl =
      new wxStaticText(systeminfoPage, wxID_ANY, distroInfo);
  wxStaticText *cpuArchitecture =
      new wxStaticText(systeminfoPage, wxID_ANY, platform.GetBitnessName());

  wxStaticText *desktopEnv =
      new wxStaticText(systeminfoPage, wxID_ANY, wxEmptyString);
  desktopEnv->SetLabelText(de);

  bottomRowLeftChild->Add(osUserNameCtrl, 0, wxALL | wxEXPAND, 5);

  bottomRowLeftChild->Add(osDistroCtrl, 0, wxALL | wxEXPAND, 5);
  bottomRowLeftChild->Add(osDescCtrl, 0, wxALL | wxEXPAND, 5);
  bottomRowLeftChild->Add(osNameCtrl, 0, wxALL | wxEXPAND, 5);
  bottomRowLeftChild->Add(cpuArchitecture, 0, wxALL | wxEXPAND, 5);
  bottomRowLeftChild->Add(desktopEnv, 0, wxALL | wxEXPAND, 5);

  //------------------------------ END OF OS INFO DETAILS
  //: LEFT-----------------------------------//

  //---------------------------- BATTERY INFORMATION :RIGHT
  //-------------------------------------//
  bottomRowRightChild->Add(
      new wxStaticText(systeminfoPage, wxID_ANY, "BATTERY INFORMATION"), 0,
      wxALL | wxALIGN_CENTRE_HORIZONTAL, 5);

  class BatteryInfoPanel : public wxPanel {
  public:
    BatteryInfoPanel(wxPanel *parent) : wxPanel(parent) {
      SetBackgroundStyle(wxBG_STYLE_PAINT);
      Bind(wxEVT_PAINT, &BatteryInfoPanel::OnPaint, this);

      m_timer = new wxTimer(this);
      Bind(wxEVT_TIMER, &BatteryInfoPanel::OnTimer, this);
      m_timer->Start(5000); // Update every 5 seconds

      FindBatteries();
      UpdateBatteryInfo();

      // Set minimum size to prevent layout issues
      SetMinSize(wxSize(450, 350));
    }

    ~BatteryInfoPanel() {
      if (m_timer) {
        m_timer->Stop();
        delete m_timer;
      }
    }

  private:
    std::vector<wxString> m_batteryPaths;
    wxString m_powerSupplyType = "Unknown";
    int m_batteryPercentage = 0;
    wxString m_batteryState = "Unknown";
    wxString m_batteryInfo = "No battery information available";
    wxTimer *m_timer = nullptr;

    double m_powerNow = 0.0;
    wxString m_technology = "Unknown";
    int m_temperature = 0;
    int m_fullCapacity = 0;
    wxString m_timeToFullOrEmpty = "N/A";
    int m_cycleCount = 0;
    int m_batteryHealth = 100;

    wxString ReadFileContent(const wxString &path) {
      wxString content;
      wxTextFile file(path);
      if (file.Open()) {
        content = file.GetFirstLine();
        file.Close();
      }
      return content;
    }

    void FindBatteries() {
      m_batteryPaths.clear();
      wxString bat0_path = "/sys/class/power_supply/BAT0/";

      if (wxDir::Exists(bat0_path)) {
        wxLogMessage("BAT0 directory found: %s", bat0_path);

        wxString typeContent = ReadFileContent(bat0_path + "type");
        if (typeContent.Trim() == "Battery") {
          wxLogMessage("Valid battery found in BAT0");
          m_batteryPaths.push_back(bat0_path);
        } else {
          wxLogWarning("BAT0 exists but type file doesn't contain 'Battery'. "
                       "Content: %s",
                       typeContent);
        }
      } else {
        wxLogError("BAT0 directory not found at: %s", bat0_path);
      }

      if (m_batteryPaths.empty()) {
        wxLogWarning("No battery found in power supply directory");
      } else {
        wxLogMessage("Found %zu batteries", m_batteryPaths.size());
      }
    }

    void UpdateBatteryInfo() {
      if (!m_batteryPaths.empty()) {
        wxString batteryPath =
            m_batteryPaths[0]; // Use the first (and only) battery found

        m_powerSupplyType = ReadFileContent(batteryPath + "type").Trim();
        m_batteryState = ReadFileContent(batteryPath + "status").Trim();

        wxString capacityStr = ReadFileContent(batteryPath + "capacity").Trim();
        if (!capacityStr.ToInt(&m_batteryPercentage)) {
          m_batteryPercentage = 0;
        }

        wxString currentNowStr =
            ReadFileContent(batteryPath + "current_now").Trim();
        double currentNow;
        if (currentNowStr.ToDouble(&currentNow)) {
          m_powerNow =
              currentNow /
              1000000.0; // Convert to watts (assuming voltage is around 1V)
        } else {
          m_powerNow = 0.0;
        }

        m_technology = ReadFileContent(batteryPath + "technology").Trim();

        wxString cycleCountStr =
            ReadFileContent(batteryPath + "cycle_count").Trim();
        if (!cycleCountStr.ToInt(&m_cycleCount)) {
          m_cycleCount = 0;
        }

        wxString chargeFullStr =
            ReadFileContent(batteryPath + "charge_full").Trim();
        wxString chargeFullDesignStr =
            ReadFileContent(batteryPath + "charge_full_design").Trim();
        int chargeFull, chargeFullDesign;
        if (chargeFullStr.ToInt(&chargeFull) &&
            chargeFullDesignStr.ToInt(&chargeFullDesign) &&
            chargeFullDesign > 0) {
          m_batteryHealth = static_cast<int>(
              (static_cast<double>(chargeFull) / chargeFullDesign) * 100);
          m_fullCapacity = chargeFull / 1000; // Convert to mAh
        } else {
          m_batteryHealth = 100;
          m_fullCapacity = 0;
        }

        // Calculate time to full/empty (simplified, might not be accurate)
        wxString chargeNowStr =
            ReadFileContent(batteryPath + "charge_now").Trim();
        int chargeNow;
        if (chargeNowStr.ToInt(&chargeNow) && m_powerNow > 0) {
          double timeHours =
              (m_batteryState == "Charging")
                  ? (chargeFull - chargeNow) / (m_powerNow * 1000000.0)
                  : chargeNow / (m_powerNow * 1000000.0);
          int hours = static_cast<int>(timeHours);
          int minutes = static_cast<int>((timeHours - hours) * 60);
          m_timeToFullOrEmpty = wxString::Format("%d:%02d", hours, minutes);
        } else {
          m_timeToFullOrEmpty = "N/A";
        }

        m_batteryInfo = wxString::Format(
            "Capacity: %d%%\n"
            "Power: %.2f W\n"
            "Technology: %s\n"
            "Full Capacity: %d mAh\n"
            "Time to %s: %s\n"
            "Cycle Count: %d\n"
            "Battery Health: %d%%",
            m_batteryPercentage, m_powerNow, m_technology, m_fullCapacity,
            (m_batteryState == "Charging" ? "Full" : "Empty"),
            m_timeToFullOrEmpty, m_cycleCount, m_batteryHealth);
      } else {
        m_powerSupplyType = "Unknown";
        m_batteryPercentage = 0;
        m_batteryState = "No battery found";
        m_batteryInfo = "No battery information available";
      }
    }

    wxColor GetBatteryColor() {
      if (m_batteryPercentage > 85) {
        return wxColor(0, 255, 0); // Green
      } else if (m_batteryPercentage > 45) {
        return wxColor(255, 165, 0); // Orange
      } else {
        return wxColor(255, 0, 0); // Red
      }
    }

    void OnPaint(wxPaintEvent &event) {
      wxAutoBufferedPaintDC dc(this);
      dc.Clear();
      wxGraphicsContext *gc = wxGraphicsContext::Create(dc);
      if (gc) {
        // Get the size of the panel
        wxSize size = GetClientSize();

        // Calculate the battery bar size and position
        const int barWidth = 50;
        const int barHeight = size.GetHeight() - 100;
        const int barX = 50;
        const int barY = 50;

        // Draw battery percentage bar
        gc->SetBrush(*wxWHITE_BRUSH);
        gc->SetPen(*wxBLACK_PEN);
        gc->DrawRectangle(barX, barY, barWidth, barHeight);

        wxColor batteryColor = GetBatteryColor();
        gc->SetBrush(wxBrush(batteryColor));
        int filledHeight =
            static_cast<int>(barHeight * m_batteryPercentage / 100.0);
        gc->DrawRectangle(barX, barY + barHeight - filledHeight, barWidth,
                          filledHeight);

        // Draw percentage text
        gc->SetFont(wxFont(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
                           wxFONTWEIGHT_NORMAL),
                    *wxBLACK);
        wxString percentText = wxString::Format("%d%%", m_batteryPercentage);
        gc->DrawText(percentText, barX, barY + barHeight + 10);

        // Draw battery info text
        int textX = barX + barWidth + 20;
        int textY = barY;

        auto drawText = [&](const wxString &text) {
          gc->DrawText(text, textX, textY);
          textY += 20;
        };

        drawText(wxString::Format("Power Supply Type: %s", m_powerSupplyType));
        drawText(wxString::Format("Battery State: %s", m_batteryState));

        // Draw the new battery information
        wxStringTokenizer tokenizer(m_batteryInfo, "\n");
        while (tokenizer.HasMoreTokens()) {
          drawText(tokenizer.GetNextToken());
        }

        delete gc;
      }
    }

    void OnTimer(wxTimerEvent &event) {
      UpdateBatteryInfo();
      Refresh(); // Request a repaint
    }
  };

  // Add BATTERY INFORMATION to bottom right sizer of page 1
  BatteryInfoPanel *batteryInfoPanel = new BatteryInfoPanel(systeminfoPage);
  bottomRowRightChild->Add(batteryInfoPanel, 1, wxEXPAND | wxALL, 5);

  //---------------------------- END OF BATTERY INFORMATION CONTENT
  //-------------------------//

  // Add widget to row sizers
  // ''''''''''''''''''''''''''''''''''''''''''''''''''''
  topRowSizer->Add(topRowLeftChild, 1, wxEXPAND);
  topRowSizer->Add(topRowCenterChild, 1, wxEXPAND);
  topRowSizer->Add(topRowRightChild, 1, wxEXPAND);
  bottomRowSizer->Add(bottomRowLeftChild, 1, wxEXPAND);
  bottomRowSizer->Add(bottomRowRightChild, 1, wxEXPAND);
  //''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
  //-------------------------------------------------------------------------------------------
  // Create main sizer for page 1
  wxBoxSizer *firstPageSizer = new wxBoxSizer(wxVERTICAL);
  firstPageSizer->Add(topRowSizer, 1, wxEXPAND, 10);
  firstPageSizer->Add(bottomRowSizer, 1, wxEXPAND, 10);
  systeminfoPage->SetSizer(firstPageSizer);
  systeminfoPage->Fit();
  //------------------------------------------------------------------------------------------
  // ************ END: END OF PAGE 1 ***************************/

  //-------------------------------------------------------------------------------------------

  /************* BEGIN: Populate page 2 values ***************/
  class ProcessInfo {
  public:
    long pid;
    wxString user;
    wxString name;
    wxString cpu;
    wxString ram;
    wxString time;
    wxString command;
  };

  class ResourcesInfoPage : public wxPanel {
  public:
    ResourcesInfoPage(wxPanel *parent) : wxPanel(parent, wxID_ANY) {
      // Main sizer for the panel
      wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);

      // Create the list control
      m_listCtrl = new wxListCtrl(this, wxID_ANY, wxDefaultPosition,
                                  wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL);

      m_listCtrl->InsertColumn(0, "PID");
      m_listCtrl->InsertColumn(1, "User");
      m_listCtrl->InsertColumn(2, "Name");
      m_listCtrl->InsertColumn(3, "CPU%");
      m_listCtrl->InsertColumn(4, "RAM%");
      m_listCtrl->InsertColumn(5, "Time");
      m_listCtrl->InsertColumn(6, "Command");
      m_listCtrl->InsertColumn(7, "Actions");

      mainSizer->Add(m_listCtrl, 1, wxEXPAND | wxALL, 5);

      // Create button sizer
      wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);

      wxButton *refreshButton = new wxButton(this, wxID_ANY, "Refresh");
      buttonSizer->Add(refreshButton, 0, wxALL, 5);

      m_totalProcessesButton =
          new wxButton(this, wxID_ANY, "Total Processes: 0");
      buttonSizer->Add(m_totalProcessesButton, 0, wxALL, 5);

      mainSizer->Add(buttonSizer, 0, wxALIGN_RIGHT);

      SetSizer(mainSizer);

      // Bind events
      refreshButton->Bind(wxEVT_BUTTON, &ResourcesInfoPage::OnRefresh, this);
      m_totalProcessesButton->Bind(
          wxEVT_BUTTON, &ResourcesInfoPage::OnShowTotalProcesses, this);
      m_listCtrl->Bind(wxEVT_LIST_ITEM_ACTIVATED,
                       &ResourcesInfoPage::OnItemActivated, this);

      RefreshProcessList();
    }

  private:
    wxListCtrl *m_listCtrl;
    wxButton *m_totalProcessesButton;
    std::vector<ProcessInfo> m_processes;

    wxString ExtractProcessName(const wxString &command) {
      wxString name = command;

      // Remove leading whitespace
      name.Trim(false);

      // Check if it's a kernel process (enclosed in square brackets)
      if (name.StartsWith("[") && name.Contains("]")) {
        size_t endBracket = name.Find("]");
        if (endBracket != wxString::npos) {
          return name.Left(endBracket + 1);
        }
      }

      // For normal processes, extract the base name of the executable
      size_t spacePos = name.Find(' ');
      if (spacePos != wxString::npos) {
        name = name.Left(spacePos);
      }

      // Remove any leading directory path
      size_t lastSlash = name.Find('/', true);
      if (lastSlash != wxString::npos) {
        name = name.Mid(lastSlash + 1);
      }

      return name;
    }

    void RefreshProcessList() {
      m_listCtrl->DeleteAllItems();
      m_processes.clear();

      wxArrayString output;
      wxExecute("ps aux --sort=-pcpu", output);

      for (size_t i = 1; i < output.GetCount(); ++i) {
        wxString line = output[i];

        // Split the line into fields
        wxStringTokenizer tokenizer(line, " ", wxTOKEN_STRTOK);
        wxArrayString fields;
        while (tokenizer.HasMoreTokens()) {
          fields.Add(tokenizer.GetNextToken());
        }

        if (fields.size() >= 11) {
          ProcessInfo info;
          info.user = fields[0];
          info.pid = wxAtol(fields[1]);
          info.cpu = fields[2] + "%";
          info.ram = fields[3] + "%";
          info.time = fields[9];

          // Combine all remaining fields for the command
          wxArrayString commandParts(fields.begin() + 10, fields.end());
          info.command = wxJoin(commandParts, ' ');

          // Extract the process name from the command
          info.name = ExtractProcessName(info.command);

          m_processes.push_back(info);

          long itemIndex = m_listCtrl->InsertItem(
              m_listCtrl->GetItemCount(), wxString::Format("%ld", info.pid));
          m_listCtrl->SetItem(itemIndex, 1, info.user);
          m_listCtrl->SetItem(itemIndex, 2, info.name);
          m_listCtrl->SetItem(itemIndex, 3, info.cpu);
          m_listCtrl->SetItem(itemIndex, 4, info.ram);
          m_listCtrl->SetItem(itemIndex, 5, info.time);
          m_listCtrl->SetItem(itemIndex, 6, info.command);
        }
      }

      for (int i = 0; i < m_listCtrl->GetColumnCount(); ++i) {
        m_listCtrl->SetColumnWidth(i, wxLIST_AUTOSIZE_USEHEADER);
      }

      UpdateTotalProcessesCount();
    }

    void UpdateTotalProcessesCount() {
      m_totalProcessesButton->SetLabel(
          wxString::Format("Total Processes: %zu", m_processes.size()));
    }

    void OnRefresh(wxCommandEvent &event) { RefreshProcessList(); }

    void OnShowTotalProcesses(wxCommandEvent &event) {
      wxMessageBox(wxString::Format("Total number of processes: %zu",
                                    m_processes.size()),
                   "Process Count", wxOK | wxICON_INFORMATION);
    }

    void OnItemActivated(wxListEvent &event) {
      long itemIndex = event.GetIndex();
      wxButton *killButton =
          reinterpret_cast<wxButton *>(m_listCtrl->GetItemData(itemIndex));
      if (killButton) {
        killButton->Show();
      }
    }

    void OnKillProcess(wxCommandEvent &event) {
      wxButton *button = dynamic_cast<wxButton *>(event.GetEventObject());
      if (button) {
        wxVariant *variant =
            reinterpret_cast<wxVariant *>(button->GetClientData());
        wxLongLong pid = variant->GetLongLong();
        wxKill(pid.ToLong(), wxSIGTERM, NULL, wxKILL_CHILDREN);
        RefreshProcessList();
      }
    }
  };

  // Create main sizer for page 2
  wxBoxSizer *secondPageSizer = new wxBoxSizer(wxVERTICAL);
  ResourcesInfoPage *resourcesPageContent =
      new ResourcesInfoPage(resourcesInfoPage);
  secondPageSizer->Add(resourcesPageContent, 1, wxEXPAND | wxALL, 5);
  resourcesInfoPage->SetSizer(secondPageSizer);

  /************ END: END OF PAGE 2 ***************************/

  /************* BEGIN: Populate page 3 values ***************/
  // Create a responsive sizer for page 3 (Miscellaneous Info)
  wxBoxSizer *miscPageSizer = new wxBoxSizer(wxVERTICAL);

  // Create top and bottom row sizers
  wxBoxSizer *miscTopRowSizer = new wxBoxSizer(wxHORIZONTAL);
  wxBoxSizer *miscBottomRowSizer = new wxBoxSizer(wxHORIZONTAL);

  // Create panes for the top row
  wxPanel *networkPane = new wxPanel(miscInfoPage, wxID_ANY);
  wxPanel *displayInfoPane = new wxPanel(miscInfoPage, wxID_ANY);
  wxPanel *storageDevicesPane = new wxPanel(miscInfoPage, wxID_ANY);

  // Create panes for the bottom row
  wxPanel *audioDevicesPane = new wxPanel(miscInfoPage, wxID_ANY);
  wxPanel *motherboardInfoPane = new wxPanel(miscInfoPage, wxID_ANY);
  wxPanel *cpuInfoPane = new wxPanel(miscInfoPage, wxID_ANY);

  // Add panes to the top row sizer
  miscTopRowSizer->Add(networkPane, 1, wxEXPAND | wxALL, 5);
  miscTopRowSizer->Add(displayInfoPane, 1, wxEXPAND | wxALL, 5);
  miscTopRowSizer->Add(storageDevicesPane, 1, wxEXPAND | wxALL, 5);

  // Add panes to the bottom row sizer
  miscBottomRowSizer->Add(audioDevicesPane, 1, wxEXPAND | wxALL, 5);
  miscBottomRowSizer->Add(motherboardInfoPane, 1, wxEXPAND | wxALL, 5);
  miscBottomRowSizer->Add(cpuInfoPane, 1, wxEXPAND | wxALL, 5);

  // ------------------------------ NETWORK INFORMATION
  // ------------------------------ Set up the network pane
  wxStaticBoxSizer *networkSizer =
      new wxStaticBoxSizer(wxVERTICAL, networkPane, "NETWORK INFORMATION");

  class NetworkInfoPanel : public wxPanel {
  public:
    NetworkInfoPanel(wxWindow *parent) : wxPanel(parent, wxID_ANY) {
      wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);

      wxBoxSizer *titleSizer = new wxBoxSizer(wxHORIZONTAL);
      statusDot =
          new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(10, 10));
      titleLabel = new wxStaticText(this, wxID_ANY, "Network Information");
      titleSizer->Add(statusDot, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
      titleSizer->Add(titleLabel, 0, wxALIGN_CENTER_VERTICAL);
      mainSizer->Add(titleSizer, 0, wxALL, 10);

      wxFlexGridSizer *gridSizer = new wxFlexGridSizer(8, 2, 10, 10);

      ipLabel = new wxStaticText(this, wxID_ANY, "IP Address:");
      ipValue = new wxStaticText(this, wxID_ANY, "");
      gridSizer->Add(ipLabel);
      gridSizer->Add(ipValue);

      macLabel = new wxStaticText(this, wxID_ANY, "MAC Address:");
      macValue = new wxStaticText(this, wxID_ANY, "");
      gridSizer->Add(macLabel);
      gridSizer->Add(macValue);

      interfaceLabel = new wxStaticText(this, wxID_ANY, "Interface:");
      interfaceValue = new wxStaticText(this, wxID_ANY, "");
      gridSizer->Add(interfaceLabel);
      gridSizer->Add(interfaceValue);

      uploadLabel = new wxStaticText(this, wxID_ANY, "Upload Speed:");
      uploadValue = new wxStaticText(this, wxID_ANY, "");
      gridSizer->Add(uploadLabel);
      gridSizer->Add(uploadValue);

      downloadLabel = new wxStaticText(this, wxID_ANY, "Download Speed:");
      downloadValue = new wxStaticText(this, wxID_ANY, "");
      gridSizer->Add(downloadLabel);
      gridSizer->Add(downloadValue);

      totalUploadLabel = new wxStaticText(this, wxID_ANY, "Total Uploaded:");
      totalUploadValue = new wxStaticText(this, wxID_ANY, "");
      gridSizer->Add(totalUploadLabel);
      gridSizer->Add(totalUploadValue);

      totalDownloadLabel =
          new wxStaticText(this, wxID_ANY, "Total Downloaded:");
      totalDownloadValue = new wxStaticText(this, wxID_ANY, "");
      gridSizer->Add(totalDownloadLabel);
      gridSizer->Add(totalDownloadValue);

      gatewayLabel = new wxStaticText(this, wxID_ANY, "Default Gateway:");
      gatewayValue = new wxStaticText(this, wxID_ANY, "");
      gridSizer->Add(gatewayLabel);
      gridSizer->Add(gatewayValue);

      mainSizer->Add(gridSizer, 0, wxALL, 10);

      SetSizer(mainSizer);

      timer = new wxTimer(this);

      Bind(wxEVT_TIMER, &NetworkInfoPanel::OnTimer, this);
      timer->Start(1000); // Update every second
    }

  private:
    wxPanel *statusDot;
    wxStaticText *titleLabel;
    wxStaticText *ipLabel;
    wxStaticText *ipValue;
    wxStaticText *macLabel;
    wxStaticText *macValue;
    wxStaticText *interfaceLabel;
    wxStaticText *interfaceValue;
    wxStaticText *uploadLabel;
    wxStaticText *uploadValue;
    wxStaticText *downloadLabel;
    wxStaticText *downloadValue;
    wxStaticText *totalUploadLabel;
    wxStaticText *totalUploadValue;
    wxStaticText *totalDownloadLabel;
    wxStaticText *totalDownloadValue;
    wxStaticText *gatewayLabel;
    wxStaticText *gatewayValue;
    wxTimer *timer;

    long long lastTxBytes = 0;
    long long lastRxBytes = 0;
    std::string currentInterface;

    void OnTimer(wxTimerEvent &event) { UpdateNetworkInfo(); }

    void UpdateNetworkInfo() {
      // Get network interface information
      std::string ipAddress, macAddress;
      GetInterfaceInfo(currentInterface, ipAddress, macAddress);

      ipValue->SetLabel(ipAddress);
      macValue->SetLabel(macAddress);
      interfaceValue->SetLabel(currentInterface);

      // Get upload and download speeds
      long long txBytes, rxBytes;
      GetNetworkUsage(currentInterface, txBytes, rxBytes);

      double uploadSpeed = (txBytes - lastTxBytes) / 1024.0;   // KB/s
      double downloadSpeed = (rxBytes - lastRxBytes) / 1024.0; // KB/s

      uploadValue->SetLabel(wxString::Format("%.2f KB/s", uploadSpeed));
      downloadValue->SetLabel(wxString::Format("%.2f KB/s", downloadSpeed));

      totalUploadValue->SetLabel(FormatBytes(txBytes));
      totalDownloadValue->SetLabel(FormatBytes(rxBytes));

      lastTxBytes = txBytes;
      lastRxBytes = rxBytes;

      // Get default gateway
      std::string gateway = GetDefaultGateway();
      gatewayValue->SetLabel(gateway);

      // Update status dot
      bool isOnline = !ipAddress.empty() && ipAddress != "0.0.0.0";
      statusDot->SetBackgroundColour(
          isOnline
              ? *wxGREEN
              : wxColour(255, 165, 0)); // Green if online, orange if offline
      statusDot->Refresh();
    }

    void GetInterfaceInfo(std::string &interfaceName, std::string &ipAddress,
                          std::string &macAddress) {
      struct ifaddrs *ifaddr, *ifa;
      int family, s;
      char host[NI_MAXHOST];

      if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        return;
      }

      for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL)
          continue;

        family = ifa->ifa_addr->sa_family;

        if (family == AF_INET) {
          s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host,
                          NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
          if (s != 0) {
            printf("getnameinfo() failed: %s\n", gai_strerror(s));
            continue;
          }

          if (strcmp(ifa->ifa_name, "lo") != 0) { // Ignore loopback interface
            interfaceName = ifa->ifa_name;
            ipAddress = host;

            // Get MAC address
            struct ifreq ifr;
            int fd = socket(AF_INET, SOCK_DGRAM, 0);
            ifr.ifr_addr.sa_family = AF_INET;
            strncpy(ifr.ifr_name, ifa->ifa_name, IFNAMSIZ - 1);
            ioctl(fd, SIOCGIFHWADDR, &ifr);
            close(fd);

            unsigned char *mac = (unsigned char *)ifr.ifr_hwaddr.sa_data;
            char macStr[18];
            snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
                     mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
            macAddress = macStr;

            break; // Use the first non-loopback interface
          }
        }
      }

      freeifaddrs(ifaddr);
    }

    void GetNetworkUsage(const std::string &interface, long long &txBytes,
                         long long &rxBytes) {
      std::ifstream statFile("/proc/net/dev");
      std::string line;
      while (std::getline(statFile, line)) {
        if (line.find(interface + ":") != std::string::npos) {
          std::istringstream iss(line);
          std::string token;
          iss >> token;   // Skip interface name
          iss >> rxBytes; // Received bytes
          for (int i = 0; i < 7; ++i)
            iss >> token; // Skip to transmitted bytes
          iss >> txBytes;
          break;
        }
      }
    }

    std::string GetDefaultGateway() {
      std::ifstream routeFile("/proc/net/route");
      std::string line;
      while (std::getline(routeFile, line)) {
        std::istringstream iss(line);
        std::string interfaceName, destination, gateway;
        iss >> interfaceName >> destination >> gateway;
        if (destination == "00000000") // Default route
        {
          unsigned int gatewayAddr;
          sscanf(gateway.c_str(), "%X", &gatewayAddr);
          struct in_addr addr;
          addr.s_addr = gatewayAddr;
          return inet_ntoa(addr);
        }
      }
      return "Not found";
    }

    std::string FormatBytes(long long bytes) {
      const char *units[] = {"B", "KB", "MB", "GB", "TB"};
      int i = 0;
      double size = bytes;
      while (size > 1024 && i < 4) {
        size /= 1024;
        i++;
      }
      char buffer[32];
      snprintf(buffer, sizeof(buffer), "%.2f %s", size, units[i]);
      return std::string(buffer);
    }
  };

  // Add NETWORK INFORMATION to the top left of the page 3
  NetworkInfoPanel *networkInfo = new NetworkInfoPanel(networkPane);
  networkSizer->Add(networkInfo, 1, wxEXPAND | wxALL, 5);
  networkPane->SetSizer(networkSizer);
  networkPane->Layout();


// ----------------- DISPLAy INFORMATION -------------
wxStaticBoxSizer *displaySizer = new wxStaticBoxSizer(
    wxVERTICAL, displayInfoPane, "DISPLAY INFORMATION");

class DisplayInfoPanel : public wxPanel {
public:
    DisplayInfoPanel(wxWindow *parent) : wxPanel(parent, wxID_ANY) {
        m_sizer = std::make_unique<wxBoxSizer>(wxVERTICAL);
        SetSizer(m_sizer.get());

        CollectDisplayInfo();
        CreateControls();

        Bind(wxEVT_SIZE, &DisplayInfoPanel::OnSize, this);
    }

private:
    std::unique_ptr<wxBoxSizer> m_sizer;
    std::vector<wxStaticText*> m_labels;
    std::vector<std::pair<std::string, std::string>> m_displayInfo;

    void CollectDisplayInfo() {
        try {
            CollectFromWxDisplay();
            CollectDisplayManager();
            CollectFromEdid();
            CollectGpuInfo();
        } catch (const std::exception& e) {
            wxLogError("Failed to collect display info: %s", e.what());
        }
    }

    void CollectFromWxDisplay() {
        int displayCount = wxDisplay::GetCount();
        if (displayCount > 0) {
            //wxDisplay display(0);  // Primary display
            wxDisplay display(static_cast<unsigned int>(0));  // Primary display

            wxVideoMode mode = display.GetCurrentMode();

            m_displayInfo.emplace_back("Screen Resolution", 
                std::format("{}x{}", mode.GetWidth(), mode.GetHeight()));
            m_displayInfo.emplace_back("Refresh Rate", 
                std::format("{} Hz", 60));  // mode.refresh =! 60
            m_displayInfo.emplace_back("Color Depth", 
                std::format("{} bits", mode.GetDepth()));
        } else {
            wxLogError("No displays found");
        }
    }

    void CollectDisplayManager() {
        wxString displayManager = "Unknown";
        wxArrayString output;
        if (wxExecute("ps -e | grep -E 'gdm|sddm|lightdm|lxdm|xdm'", output, wxEXEC_NODISABLE) == 0) {
            if (!output.IsEmpty()) {
                wxString line = output[0];
                if (line.Contains("gdm")) displayManager = "GDM";
                else if (line.Contains("sddm")) displayManager = "SDDM";
                else if (line.Contains("lightdm")) displayManager = "LightDM";
                else if (line.Contains("lxdm")) displayManager = "LXDM";
                else if (line.Contains("xdm")) displayManager = "XDM";
            }
        }
        //displayManager.ToStdString()
        m_displayInfo.emplace_back("Display Manager", "GDM");
    }

    void CollectFromEdid() {
        try {
            const std::filesystem::path edidPath("/sys/class/drm");
            for (const auto& entry : std::filesystem::directory_iterator(edidPath)) {
                std::string filename = entry.path().string() + "/edid";
                if (std::filesystem::exists(filename)) {
                    std::ifstream file(filename, std::ios::binary);
                    if (file) {
                        std::vector<unsigned char> buffer(128);
                        if (file.read(reinterpret_cast<char*>(buffer.data()), 128)) {
                            std::string manufacturer;
                            manufacturer += static_cast<char>(((buffer[8] & 0x7C) >> 2) + 'A' - 1);
                            manufacturer += static_cast<char>(((buffer[8] & 0x03) << 3 | (buffer[9] & 0xE0) >> 5) + 'A' - 1);
                            manufacturer += static_cast<char>((buffer[9] & 0x1F) + 'A' - 1);
                            
                            std::string model;
                            for (int i = 54; i <= 71; ++i) {
                                if (buffer[i] == 0x0A) break;
                                if (buffer[i] >= 32 && buffer[i] <= 126) {
                                    model += static_cast<char>(buffer[i]);
                                }
                            }
                            
                            m_displayInfo.emplace_back("Monitor Manufacturer", manufacturer);
                            m_displayInfo.emplace_back("Monitor Model", model);
                            return;
                        }
                    }
                }
            }
        } catch (const std::filesystem::filesystem_error& e) {
            wxLogError("Permission denied accessing EDID: %s", e.what());
        }
    }

    void CollectGpuInfo() {
        wxArrayString output;
        if (wxExecute("lspci | grep -i vga", output, wxEXEC_NODISABLE) == 0 && !output.IsEmpty()) {
            m_displayInfo.emplace_back("GPU Model", output[0].AfterFirst(':').Trim().ToStdString());
        }

        std::ifstream file("/sys/class/drm/card0/device/driver/module/drivers/pci:*/module/version");
        if (file) {
            std::string version;
            if (std::getline(file, version)) {
                m_displayInfo.emplace_back("GPU Driver Version", version);
            }
        }
    }

    void CreateControls() {
        for (const auto& info : m_displayInfo) {
            auto* label = new wxStaticText(this, wxID_ANY, 
                wxString::Format("%s: %s", info.first, info.second));
            m_labels.push_back(label);
            m_sizer->Add(label, 0, wxEXPAND | wxALL, 5);
        }
    }

    void OnSize(wxSizeEvent& event) {
        int panelWidth = GetSize().GetWidth();
        int fontSize = std::max(8, std::min(16, panelWidth / 50));

        wxFont font = GetFont();
        font.SetPointSize(fontSize);

        for (auto* label : m_labels) {
            label->SetFont(font);
        }

        event.Skip();
    }
};


// Add NETWORK INFORMATION to the top left of the page 3
  DisplayInfoPanel *displayInfo = new DisplayInfoPanel(displayInfoPane);
  displaySizer->Add(displayInfo, 1, wxEXPAND | wxALL, 10);
  displayInfoPane->SetSizer(displaySizer);
  displayInfoPane->Layout();
// ------------------ DISPLAY END =====================



// ---------------------------- STORAGE DEVICES
// ---------------------------------------------
wxStaticBoxSizer *storageSizer = new wxStaticBoxSizer(
    wxVERTICAL, storageDevicesPane, "STORAGE INFORMATION");

class CustomGauge : public wxPanel {
public:
  CustomGauge(wxWindow *parent, wxWindowID id = wxID_ANY, int range = 100,
              wxPoint pos = wxDefaultPosition, wxSize size = wxDefaultSize)
      : wxPanel(parent, id, pos, size), m_range(range), m_value(0) {
    Bind(wxEVT_PAINT, &CustomGauge::OnPaint, this);
  }

  void SetValue(int value) {
    m_value = value;
    Refresh();
  }

private:
  void OnPaint(wxPaintEvent &event) {
    wxPaintDC dc(this);
    wxSize size = GetSize();

    // Draw background (free space) in green
    dc.SetBrush(wxBrush(wxColor(0, 255, 0)));
    dc.DrawRectangle(0, 0, size.GetWidth(), size.GetHeight());

    // Draw foreground (used space) in brown
    int usedWidth = static_cast<int>(
        (static_cast<double>(m_value) / m_range) * size.GetWidth());
    dc.SetBrush(wxBrush(wxColor(165, 42, 42)));
    dc.DrawRectangle(0, 0, usedWidth, size.GetHeight());
  }

  int m_range;
  int m_value;
};

class StorageDevicesPanel : public wxPanel {
public:
  StorageDevicesPanel(wxWindow *parent, wxWindowID id = wxID_ANY)
      : wxPanel(parent, id) {
    wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);

    wxLogMessage("StorageDevicesPanel constructor called");

    std::vector<StorageInfo> storageInfos = getStorageDevices();

    wxLogMessage(
        wxString::Format("Found %zu storage devices", storageInfos.size()));

    if (storageInfos.empty()) {
      wxStaticText *errorText =
          new wxStaticText(this, wxID_ANY,
                           "No storage devices found or unable to retrieve "
                           "storage information.");
      mainSizer->Add(errorText, 0, wxALL, 5);
    } else {
      for (const auto &info : storageInfos) {
        wxBoxSizer *deviceSizer = new wxBoxSizer(wxVERTICAL);

        wxStaticText *nameText = new wxStaticText(this, wxID_ANY, info.name);
        deviceSizer->Add(nameText, 0, wxEXPAND | wxBOTTOM, 5);

        CustomGauge *gauge = new CustomGauge(
            this, wxID_ANY, 100, wxDefaultPosition, wxSize(-1, 20));
        int usedPercentage =
            static_cast<int>(std::round(info.usedPercentage));
        gauge->SetValue(usedPercentage);
        deviceSizer->Add(gauge, 0, wxEXPAND | wxBOTTOM, 5);

        wxString storageText = wxString::Format(
            "Total: %.2f GB   Used: %.2f GB   Free: %.2f GB   (%.1f%% used)",
            info.totalGB, info.usedGB, info.freeGB, info.usedPercentage);
        wxStaticText *spaceText =
            new wxStaticText(this, wxID_ANY, storageText);
        deviceSizer->Add(spaceText, 0, wxEXPAND | wxBOTTOM, 10);

        mainSizer->Add(deviceSizer, 0, wxEXPAND | wxALL, 5);

        wxLogMessage(
            wxString::Format("Added device: %s, Total: %.2f GB, Used: %.2f "
                             "GB, Free: %.2f GB, Used%%: %.1f%%",
                             info.name, info.totalGB, info.usedGB,
                             info.freeGB, info.usedPercentage));
      }
    }

    SetSizer(mainSizer);
    mainSizer->Fit(this);

    wxLogMessage(wxString::Format("Panel size: %d x %d", GetSize().GetWidth(),
                                  GetSize().GetHeight()));
  }

private:
  struct StorageInfo {
    wxString name;
    double totalGB;
    double usedGB;
    double freeGB;
    double usedPercentage;
  };

  std::vector<StorageInfo> getStorageDevices() {
    std::vector<StorageInfo> devices;

    wxLogMessage("Current working directory: " + wxGetCwd());

    std::ifstream mounts("/proc/mounts");
    if (!mounts.is_open()) {
      wxLogMessage("Failed to open /proc/mounts directly. Error: " +
                   wxString(strerror(errno)));
      return getFallbackStorageInfo();
    }

    std::string line;
    while (std::getline(mounts, line)) {
      std::istringstream iss(line);
      std::string device, mountPoint;
      if (iss >> device >> mountPoint) {
        if (device.substr(0, 5) == "/dev/" &&
            mountPoint.substr(0, 4) != "/sys" &&
            mountPoint.substr(0, 5) != "/proc" &&
            mountPoint.substr(0, 4) != "/run") {

          struct statvfs stat;
          if (statvfs(mountPoint.c_str(), &stat) == 0) {
            double totalBytes =
                static_cast<double>(stat.f_frsize) * stat.f_blocks;
            double freeBytes =
                static_cast<double>(stat.f_frsize) * stat.f_bfree;
            double usedBytes = totalBytes - freeBytes;

            StorageInfo info;
            info.name = wxString::Format("%s (%s)", device, mountPoint);
            info.totalGB = totalBytes / (1024.0 * 1024.0 * 1024.0);
            info.usedGB = usedBytes / (1024.0 * 1024.0 * 1024.0);
            info.freeGB = freeBytes / (1024.0 * 1024.0 * 1024.0);
            info.usedPercentage = (usedBytes / totalBytes) * 100.0;

            devices.push_back(info);

            wxLogMessage(wxString::Format("Found device: %s, Total: %.2f GB, "
                                          "Used: %.2f GB, Free: %.2f GB",
                                          info.name, info.totalGB,
                                          info.usedGB, info.freeGB));
          } else {
            wxLogMessage(wxString::Format("statvfs failed for %s. Error: %s",
                                          mountPoint, strerror(errno)));
          }
        }
      }
    }

    if (devices.empty()) {
      wxLogMessage(
          "No devices found in /proc/mounts, using fallback method.");
      return getFallbackStorageInfo();
    }

    return devices;
  }

  std::vector<StorageInfo> getFallbackStorageInfo() {
    std::vector<StorageInfo> devices;

    std::vector<std::string> commonMountPoints = {"/", "/home"};

    for (const auto &mountPoint : commonMountPoints) {
      struct statvfs stat;
      if (statvfs(mountPoint.c_str(), &stat) == 0) {
        double totalBytes =
            static_cast<double>(stat.f_frsize) * stat.f_blocks;
        double freeBytes = static_cast<double>(stat.f_frsize) * stat.f_bfree;
        double usedBytes = totalBytes - freeBytes;

        StorageInfo info;
        info.name = wxString::Format("Fallback (%s)", mountPoint);
        info.totalGB = totalBytes / (1024.0 * 1024.0 * 1024.0);
        info.usedGB = usedBytes / (1024.0 * 1024.0 * 1024.0);
        info.freeGB = freeBytes / (1024.0 * 1024.0 * 1024.0);
        info.usedPercentage = (usedBytes / totalBytes) * 100.0;

        devices.push_back(info);

        wxLogMessage(wxString::Format("Fallback: Found device: %s, Total: "
                                      "%.2f GB, Used: %.2f GB, Free: %.2f GB",
                                      info.name, info.totalGB, info.usedGB,
                                      info.freeGB));
      } else {
        wxLogMessage(
            wxString::Format("Fallback: statvfs failed for %s. Error: %s",
                             mountPoint, strerror(errno)));
      }
    }

    return devices;
  }
};

StorageDevicesPanel *storageDevices =
    new StorageDevicesPanel(storageDevicesPane);
storageSizer->Add(storageDevices, 1, wxEXPAND | wxALL, 5);
storageDevicesPane->SetSizer(storageSizer);
storageDevicesPane->Layout();

//----------------------------- AUDIO DEVICES INFORMATION
//----------------------------------
wxStaticBoxSizer *audioSizer =
    new wxStaticBoxSizer(wxVERTICAL, audioDevicesPane, "AUDIO INFORMATION");

class AudioDevicesPanel : public wxPanel {
public:
  AudioDevicesPanel(wxWindow *parent, wxWindowID id = wxID_ANY)
      : wxPanel(parent, id) {
    wxBoxSizer *outerSizer = new wxBoxSizer(wxVERTICAL);

    // Create a scrolled window to contain the existing content
    wxScrolledWindow *scrolledWindow = new wxScrolledWindow(
        this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL);
    scrolledWindow->SetScrollRate(0, 5);

    wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);

    wxLogMessage("AudioDevicesPanel constructor called");

    std::vector<AudioDeviceInfo> audioDevices = getAudioDevices();

    wxLogMessage(
        wxString::Format("Found %zu audio devices", audioDevices.size()));

    if (audioDevices.empty()) {
      wxStaticText *errorText = new wxStaticText(
          scrolledWindow, wxID_ANY,
          "No audio devices found or unable to retrieve audio information.");
      mainSizer->Add(errorText, 0, wxALL, 5);
    } else {
      for (const auto &device : audioDevices) {
        wxStaticBoxSizer *deviceSizer =
            new wxStaticBoxSizer(wxVERTICAL, scrolledWindow, device.name);

        wxString typeStr = device.isPlayback ? "Playback" : "Capture";
        deviceSizer->Add(
            new wxStaticText(scrolledWindow, wxID_ANY, "Type: " + typeStr), 0,
            wxALL, 2);
        deviceSizer->Add(
            new wxStaticText(scrolledWindow, wxID_ANY,
                             "Description: " + device.description),
            0, wxALL, 2);

        wxString formatStr = wxString::Format(
            "Format: %s, %d channels, %d Hz", device.sampleFormat.c_str(),
            device.channels, device.sampleRate);
        deviceSizer->Add(
            new wxStaticText(scrolledWindow, wxID_ANY, formatStr), 0, wxALL,
            2);

        mainSizer->Add(deviceSizer, 0, wxEXPAND | wxALL, 5);

        wxLogMessage(wxString::Format(
            "Added device: %s, Type: %s, Channels: %d, Sample Rate: %d Hz",
            device.name, typeStr, device.channels, device.sampleRate));
      }
    }

    scrolledWindow->SetSizer(mainSizer);
    scrolledWindow->FitInside(); // Ensure the scrolled window content fits

    outerSizer->Add(scrolledWindow, 1, wxEXPAND);
    SetSizer(outerSizer);

    wxLogMessage(wxString::Format("Panel size: %d x %d", GetSize().GetWidth(),
                                  GetSize().GetHeight()));
  }

private:
  struct AudioDeviceInfo {
    wxString name;
    wxString description;
    bool isPlayback;
    int channels;
    int sampleRate;
    wxString sampleFormat;
  };

  std::vector<AudioDeviceInfo> getAudioDevices() {
    std::vector<AudioDeviceInfo> devices;

    int cardNum = -1;
    while (snd_card_next(&cardNum) >= 0 && cardNum >= 0) {
      char *cardName;
      if (snd_card_get_name(cardNum, &cardName) < 0) {
        continue;
      }

      snd_ctl_t *cardHandle;
      char cardId[32];
      snprintf(cardId, sizeof(cardId), "hw:%d", cardNum);
      if (snd_ctl_open(&cardHandle, cardId, 0) < 0) {
        continue;
      }

      int deviceNum = -1;
      while (snd_ctl_pcm_next_device(cardHandle, &deviceNum) >= 0 &&
             deviceNum >= 0) {
        for (int isPlayback = 0; isPlayback <= 1; ++isPlayback) {
          snd_pcm_info_t *pcmInfo;
          snd_pcm_info_alloca(&pcmInfo);
          snd_pcm_info_set_device(pcmInfo, deviceNum);
          snd_pcm_info_set_subdevice(pcmInfo, 0);
          snd_pcm_info_set_stream(pcmInfo, isPlayback
                                             ? SND_PCM_STREAM_PLAYBACK
                                             : SND_PCM_STREAM_CAPTURE);

          if (snd_ctl_pcm_info(cardHandle, pcmInfo) < 0) {
            continue;
          }

          AudioDeviceInfo info;
          info.name = wxString::Format("%s (Card %d, Device %d)", cardName,
                                       cardNum, deviceNum);
          info.description = wxString(snd_pcm_info_get_name(pcmInfo));
          info.isPlayback = isPlayback;

          // Get device capabilities
          snd_pcm_t *pcmHandle;
          snd_pcm_hw_params_t *hwParams;
          char deviceIdStr[64];
          snprintf(deviceIdStr, sizeof(deviceIdStr), "hw:%d,%d", cardNum,
                   deviceNum);

          if (snd_pcm_open(&pcmHandle, deviceIdStr,
                           isPlayback ? SND_PCM_STREAM_PLAYBACK
                                       : SND_PCM_STREAM_CAPTURE,
                           SND_PCM_NONBLOCK) >= 0) {
            snd_pcm_hw_params_alloca(&hwParams);
            snd_pcm_hw_params_any(pcmHandle, hwParams);

            unsigned int maxChannels;
            snd_pcm_hw_params_get_channels_max(hwParams, &maxChannels);
            info.channels = maxChannels;

            unsigned int rate = 44100; // Default to 44.1kHz
            int dir = 0;
            snd_pcm_hw_params_get_rate(hwParams, &rate, &dir);
            info.sampleRate = rate;

            snd_pcm_format_mask_t *formatMask;
            snd_pcm_format_mask_alloca(&formatMask);
            snd_pcm_hw_params_get_format_mask(hwParams, formatMask);

            if (snd_pcm_format_mask_test(formatMask, SND_PCM_FORMAT_FLOAT)) {
              info.sampleFormat = "Float";
            } else if (snd_pcm_format_mask_test(formatMask,
                                                SND_PCM_FORMAT_S32)) {
              info.sampleFormat = "32-bit";
            } else if (snd_pcm_format_mask_test(formatMask,
                                                SND_PCM_FORMAT_S24)) {
              info.sampleFormat = "24-bit";
            } else if (snd_pcm_format_mask_test(formatMask,
                                                SND_PCM_FORMAT_S16)) {
              info.sampleFormat = "16-bit";
            } else {
              info.sampleFormat = "Unknown";
            }

            snd_pcm_close(pcmHandle);
          }

          devices.push_back(info);
          wxLogMessage(wxString::Format("Found audio device: %s", info.name));
        }
      }
      snd_ctl_close(cardHandle);
    }

    return devices;
  }
};

AudioDevicesPanel *audioDevices = new AudioDevicesPanel(audioDevicesPane);
audioSizer->Add(audioDevices, 1, wxEXPAND | wxALL, 5);
audioDevicesPane->SetSizer(audioSizer);
audioDevicesPane->Layout();

//----------------------- MOTHERBOARD INFORMATION
//---------------------------------
wxStaticBoxSizer *motherBoardSizer = new wxStaticBoxSizer(
    wxVERTICAL, motherboardInfoPane, "MOTHERBOARD INFORMATION");

class MotherboardInfoPanel : public wxScrolledWindow {
public:
  MotherboardInfoPanel(wxWindow *parent, wxWindowID id = wxID_ANY)
      : wxScrolledWindow(parent, id, wxDefaultPosition, wxDefaultSize,
                         wxVSCROLL) {
    // Remove the background color setting

    m_mainSizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(m_mainSizer);

    PopulateMotherboardInfo();

    SetScrollRate(0, 10);
    Bind(wxEVT_SIZE, &MotherboardInfoPanel::OnSize, this);
  }

private:
  wxBoxSizer *m_mainSizer;
  std::vector<wxStaticText *> m_labels;

  void PopulateMotherboardInfo() {
    std::vector<std::pair<wxString, wxString>> info = GetMotherboardInfo();

    for (const auto &[key, value] : info) {
      wxBoxSizer *rowSizer = new wxBoxSizer(wxVERTICAL);

      wxStaticText *keyLabel = new wxStaticText(this, wxID_ANY, key + ":");
      keyLabel->SetFont(keyLabel->GetFont().Bold());
      rowSizer->Add(keyLabel, 0, wxEXPAND | wxALL, 5);

      wxStaticText *valueLabel =
          new wxStaticText(this, wxID_ANY, value, wxDefaultPosition,
                           wxDefaultSize, wxST_NO_AUTORESIZE);
      valueLabel->Wrap(-1); // Enable text wrapping
      rowSizer->Add(valueLabel, 0, wxEXPAND | wxALL, 5);

      m_mainSizer->Add(rowSizer, 0, wxEXPAND | wxALL, 5);
      m_labels.push_back(keyLabel);
      m_labels.push_back(valueLabel);
    }
  }

  std::vector<std::pair<wxString, wxString>> GetMotherboardInfo() {
    std::vector<std::pair<wxString, wxString>> info;

    // Read information from /sys/devices/virtual/dmi/id/
    const std::filesystem::path dmiPath("/sys/devices/virtual/dmi/id/");
    if (std::filesystem::exists(dmiPath)) {
      AddInfoFromFile(info, dmiPath / "board_vendor", "Manufacturer");
      AddInfoFromFile(info, dmiPath / "board_name", "Model");
      AddInfoFromFile(info, dmiPath / "board_version", "Version");
      AddInfoFromFile(info, dmiPath / "bios_version", "BIOS Version");
      AddInfoFromFile(info, dmiPath / "bios_date", "BIOS Date");
    }

    // // Add CPU information
    // AddInfoFromCommand(info, "lscpu | grep 'Model name'", "CPU");

    // // Add RAM information
    // AddInfoFromCommand(info, "free -h | awk '/^Mem:/ {print $2}'",
    //                    "Total RAM");

    return info;
  }

  void AddInfoFromFile(std::vector<std::pair<wxString, wxString>> &info,
                       const std::filesystem::path &path,
                       const wxString &key) {
    if (std::filesystem::exists(path)) {
      std::ifstream file(path);
      std::string value;
      std::getline(file, value);
      info.emplace_back(key, wxString(value));
    }
  }

  void AddInfoFromCommand(std::vector<std::pair<wxString, wxString>> &info,
                          const std::string &command, const wxString &key) {
    FILE *pipe = popen(command.c_str(), "r");
    if (pipe) {
      char buffer[128];
      std::string result;
      while (!feof(pipe)) {
        if (fgets(buffer, 128, pipe) != nullptr)
          result += buffer;
      }
      pclose(pipe);
      result.erase(0, result.find_first_not_of(" \n\r\t"));
      result.erase(result.find_last_not_of(" \n\r\t") + 1);
      info.emplace_back(key, wxString(result));
    }
  }

  void OnSize(wxSizeEvent &event) {
    Layout();
    for (auto *label : m_labels) {
      label->Wrap(GetSize().GetWidth() -
                  20); // Adjust wrap width based on panel size
    }
    FitInside();
    event.Skip();
  }
};

MotherboardInfoPanel *motherboardInfo =
    new MotherboardInfoPanel(motherboardInfoPane);
motherBoardSizer->Add(motherboardInfo, 1, wxEXPAND | wxALL, 5);
motherboardInfoPane->SetSizer(motherBoardSizer);
motherboardInfoPane->Layout();

//----------------------- CPUI NFORMATION ---------------------------------
wxStaticBoxSizer *cpuInfoSizer =
    new wxStaticBoxSizer(wxVERTICAL, cpuInfoPane, "CPU INFORMATION");

class CustomCPUCoreGauge : public wxPanel {
public:
  CustomCPUCoreGauge(wxWindow *parent, wxWindowID id = wxID_ANY,
                     int range = 100, const wxPoint &pos = wxDefaultPosition,
                     const wxSize &size = wxDefaultSize)
      : wxPanel(parent, id, pos, size), m_range(range), m_value(0) {
    Bind(wxEVT_PAINT, &CustomCPUCoreGauge::OnPaint, this);
  }

  void SetValue(int value) {
    m_value = value;
    Refresh();
  }

private:
  int m_range;
  int m_value;

  void OnPaint(wxPaintEvent &event) {
    wxPaintDC dc(this);
    wxSize size = GetClientSize();
    int width = size.GetWidth();
    int height = size.GetHeight();

    // Draw background (smoke-white)
    dc.SetBrush(wxBrush(wxColour(245, 245, 245)));
    dc.DrawRectangle(0, 0, width, height);

    // Draw foreground (blue)
    int fillWidth =
        static_cast<int>((static_cast<double>(m_value) / m_range) * width);
    dc.SetBrush(wxBrush(wxColour(0, 120, 215)));
    dc.DrawRectangle(0, 0, fillWidth, height);
  }
};

class CPUDetailsPanel : public wxScrolledWindow {
  public:
    CPUDetailsPanel(wxWindow *parent, wxWindowID id = wxID_ANY)
        : wxScrolledWindow(parent, id) {
      m_mainSizer = new wxBoxSizer(wxVERTICAL);
      SetSizer(m_mainSizer);

      PopulateCPUInfo();
      CreateCoreGauges();

      m_timer.Bind(wxEVT_TIMER, &CPUDetailsPanel::OnTimer, this);
      m_timer.Start(1000); // Update every second

      SetScrollRate(0, 10); // Enable vertical scrolling
      Bind(wxEVT_SIZE, &CPUDetailsPanel::OnSize, this);
    }

  private:
    wxBoxSizer *m_mainSizer;
    std::vector<CustomCPUCoreGauge *> m_coreGauges;
    std::vector<wxStaticText *> m_coreLabels;
    wxTimer m_timer;
    int m_numCores;

    void PopulateCPUInfo() {
      std::vector<std::pair<wxString, wxString>> info = GetCPUInfo();
      for (const auto &[key, value] : info) {
        wxBoxSizer *rowSizer = new wxBoxSizer(wxHORIZONTAL);
        wxStaticText *keyLabel = new wxStaticText(this, wxID_ANY, key + ":");
        keyLabel->SetFont(keyLabel->GetFont().Bold());
        wxStaticText *valueLabel = new wxStaticText(this, wxID_ANY, value);

        rowSizer->Add(keyLabel, 0, wxALL, 5);
        rowSizer->Add(valueLabel, 1, wxALL, 5);

        m_mainSizer->Add(rowSizer, 0, wxEXPAND | wxALL, 5);
      }
    }

    std::vector<std::pair<wxString, wxString>> GetCPUInfo() {
      std::vector<std::pair<wxString, wxString>> info;
      std::ifstream cpuinfo("/proc/cpuinfo");
      std::string line;
      std::string model_name, vendor_id, cpu_family;
      m_numCores = 0;

      while (std::getline(cpuinfo, line)) {
        std::istringstream iss(line);
        std::string key, value;
        if (std::getline(iss, key, ':') && std::getline(iss, value)) {
          key = Trim(key);
          value = Trim(value);
          if (key == "model name" && model_name.empty())
            model_name = value;
          if (key == "vendor_id" && vendor_id.empty())
            vendor_id = value;
          if (key == "cpu family" && cpu_family.empty())
            cpu_family = value;
          if (key == "processor")
            m_numCores++;
        }
      }

      info.emplace_back("Manufacturer", vendor_id);
      info.emplace_back("Model", model_name);
      info.emplace_back("Architecture", cpu_family + "-bit");
      info.emplace_back("Number of Cores", std::to_string(m_numCores));

      return info;
    }

    void CreateCoreGauges() {
      wxBoxSizer *coresSizer = new wxBoxSizer(wxVERTICAL);
      for (int i = 0; i < m_numCores; ++i) {
        wxBoxSizer *coreSizer = new wxBoxSizer(wxHORIZONTAL);
        wxStaticText *coreLabel = new wxStaticText(
            this, wxID_ANY, wxString::Format("Core %d: 0%%", i));
        CustomCPUCoreGauge *coreGauge = new CustomCPUCoreGauge(
            this, wxID_ANY, 100, wxDefaultPosition, wxSize(-1, 20));

        coreSizer->Add(coreLabel, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);
        coreSizer->Add(coreGauge, 1, wxEXPAND);

        coresSizer->Add(coreSizer, 0, wxEXPAND | wxALL, 5);

        m_coreGauges.push_back(coreGauge);
        m_coreLabels.push_back(coreLabel);
      }
      m_mainSizer->Add(coresSizer, 1, wxEXPAND | wxALL, 5);
    }

    void OnTimer(wxTimerEvent &event) {
      std::vector<double> cpuUsage = GetCPUUsage();
      for (size_t i = 0; i < cpuUsage.size() && i < m_coreGauges.size(); ++i) {
        int usage = static_cast<int>(cpuUsage[i] * 100);
        m_coreGauges[i]->SetValue(usage);
        m_coreLabels[i]->SetLabel(wxString::Format("Core %zu: %d%%", i, usage));
      }
    }

    std::vector<double> GetCPUUsage() {
      static std::vector<unsigned long long> prevIdle, prevTotal;
      std::vector<double> usage;
      std::ifstream statFile("/proc/stat");
      std::string line;

      while (std::getline(statFile, line)) {
        if (line.compare(0, 3, "cpu") == 0 && line[3] != ' ') {
          std::istringstream iss(line);
          std::string cpu;
          unsigned long long user, nice, system, idle, iowait, irq, softirq,
              steal, guest, guest_nice;
          iss >> cpu >> user >> nice >> system >> idle >> iowait >> irq >>
              softirq >> steal >> guest >> guest_nice;

          unsigned long long total =
              user + nice + system + idle + iowait + irq + softirq + steal;
          unsigned long long idleAllTime = idle + iowait;

          if (prevIdle.size() <= usage.size()) {
            prevIdle.push_back(0);
            prevTotal.push_back(0);
          }

          double cpuUsage =
              (1.0 - static_cast<double>(idleAllTime - prevIdle[usage.size()]) /
                         (total - prevTotal[usage.size()]));

          usage.push_back(cpuUsage);
          prevIdle[usage.size() - 1] = idleAllTime;
          prevTotal[usage.size() - 1] = total;
        }
      }

      return usage;
    }

    void OnSize(wxSizeEvent &event) {
      Layout();
      FitInside(); // Ensure scroll area is updated
      event.Skip();
    }

    static std::string Trim(const std::string &str) {
      size_t first = str.find_first_not_of(" \t");
      if (first == std::string::npos)
        return "";
      size_t last = str.find_last_not_of(" \t");
      return str.substr(first, (last - first + 1));
    }
  };

  CPUDetailsPanel *cpuDetails = new CPUDetailsPanel(cpuInfoPane);
  cpuInfoSizer->Add(cpuDetails, 1, wxEXPAND | wxALL, 5);
  cpuInfoPane->SetSizer(cpuInfoSizer);
  cpuInfoPane->Layout();

  // Add the row sizers to the main sizer
  miscPageSizer->Add(miscTopRowSizer, 1, wxEXPAND);
  miscPageSizer->Add(miscBottomRowSizer, 1, wxEXPAND);

  // Set the sizer for the miscInfoPage
  miscInfoPage->SetSizer(miscPageSizer);

  /************ END: END OF PAGE 3 ***************************/

  /************* BEGIN: Populate page 4 values ***************/
  wxStaticBoxSizer *appInfoSizer = new wxStaticBoxSizer(
      wxVERTICAL, appsInfoPage, "INSTALLED APPLICATIONS INFORMATION");

  class ApplicationInfo {
  public:
    wxString name;
    wxString version;
    wxString description;
    wxString architecture;
    wxString licenses;
    wxString size;
    wxString installReason;
    wxString dependsOn;
    wxString installDate;
    wxString url;
  };

  class ApplicationsPanel : public wxPanel {
  public:
    ApplicationsPanel(wxWindow *parent) : wxPanel(parent, wxID_ANY) {
      try {
        CreateControls();
        PopulateList();
      } catch (const std::exception &e) {
        wxLogError("Failed to initialize ApplicationsPanel: %s", e.what());
        throw;
      }
    }

  private:
    wxListCtrl *m_listCtrl;
    wxStaticText *m_statusText;

    void CreateControls() {
      wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);

      // Title
      wxStaticText *title = new wxStaticText(this, wxID_ANY, "My APPLICATIONS");
      if (!title) {
        throw std::runtime_error("Failed to create title static text");
      }
      title->SetFont(title->GetFont().Scale(1.5).Bold());
      mainSizer->Add(title, 0, wxALIGN_CENTER | wxALL, 10);

      // List control
      m_listCtrl =
          new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                         wxLC_REPORT | wxLC_HRULES | wxLC_VRULES);
      if (!m_listCtrl) {
        throw std::runtime_error("Failed to create list control");
      }
      mainSizer->Add(m_listCtrl, 1, wxEXPAND | wxALL, 5);

      // Add columns
      wxArrayString columns = {
          "Name", "Version",        "Description", "Architecture", "Licenses",
          "Size", "Install Reason", "Depends On",  "Install Date", "URL"};
      for (size_t i = 0; i < columns.size(); ++i) {
        if (m_listCtrl->InsertColumn(i, columns[i]) == -1) {
          throw std::runtime_error("Failed to insert column: " +
                                   columns[i].ToStdString());
        }
      }

      // Status text
      m_statusText = new wxStaticText(this, wxID_ANY, "");
      if (!m_statusText) {
        throw std::runtime_error("Failed to create status text");
      }
      mainSizer->Add(m_statusText, 0, wxALIGN_LEFT | wxALL, 5);

      SetSizer(mainSizer);

      // Bind events
      Bind(wxEVT_SIZE, &ApplicationsPanel::OnSize, this);
    }

    void PopulateList() {
      try {
        std::vector<ApplicationInfo> apps = GetInstalledApplications();

        m_listCtrl->Freeze();

        if (!m_listCtrl->DeleteAllItems()) {
          throw std::runtime_error("Failed to clear list control");
        }

        for (size_t i = 0; i < apps.size(); ++i) {
          const auto &app = apps[i];
          long index = m_listCtrl->InsertItem(i, app.name);
          if (index == -1) {
            throw std::runtime_error("Failed to insert item into list control");
          }

          if (!m_listCtrl->SetItem(index, 1, app.version) ||
              !m_listCtrl->SetItem(index, 2, app.description) ||
              !m_listCtrl->SetItem(index, 3, app.architecture) ||
              !m_listCtrl->SetItem(index, 4, app.licenses) ||
              !m_listCtrl->SetItem(index, 5, app.size) ||
              !m_listCtrl->SetItem(index, 6, app.installReason) ||
              !m_listCtrl->SetItem(index, 7, app.dependsOn) ||
              !m_listCtrl->SetItem(index, 8, app.installDate) ||
              !m_listCtrl->SetItem(index, 9, app.url)) {
            throw std::runtime_error("Failed to set item in list control");
          }

          // Set alternating row colors
          wxColour color =
              (i % 2 == 0) ? wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX)
                           : wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE);
          m_listCtrl->SetItemBackgroundColour(index, color);
        }

        m_listCtrl->Thaw();

        // Update status text
        int totalDeps = 0;
        for (const auto &app : apps) {
          totalDeps += wxStringTokenizer(app.dependsOn, " ").CountTokens();
        }
        wxString statusText =
            wxString::Format("Total applications: %zu, Total dependencies: %d",
                             apps.size(), totalDeps);
        m_statusText->SetLabel(statusText);

        AdjustColumnWidths();
      } catch (const std::exception &e) {
        wxLogError("Failed to populate list: %s", e.what());
        m_statusText->SetLabel("Error: Failed to load application data");
      }
    }

    std::vector<ApplicationInfo> GetInstalledApplications() {
      std::vector<ApplicationInfo> apps;
      wxArrayString output, errors;

      long exitCode = wxExecute("pacman -Qi", output, errors, wxEXEC_SYNC);
      if (exitCode != 0) {
        wxString errorMsg = "Failed to execute 'pacman -Qi'. Exit code: " +
                            wxString::Format("%ld", exitCode);
        if (!errors.IsEmpty()) {
          errorMsg += "\nErrors: " + wxJoin(errors, '\n');
        }
        throw std::runtime_error(errorMsg.ToStdString());
      }

      ApplicationInfo currentApp;
      for (const auto &line : output) {
        if (line.StartsWith("Name ")) {
          if (!currentApp.name.IsEmpty()) {
            apps.push_back(currentApp);
            currentApp = ApplicationInfo();
          }
          currentApp.name = line.AfterFirst(':').Trim();
        } else if (line.StartsWith("Version ")) {
          currentApp.version = line.AfterFirst(':').Trim();
        } else if (line.StartsWith("Description ")) {
          currentApp.description = line.AfterFirst(':').Trim();
        } else if (line.StartsWith("Architecture ")) {
          currentApp.architecture = line.AfterFirst(':').Trim();
        } else if (line.StartsWith("Licenses ")) {
          currentApp.licenses = line.AfterFirst(':').Trim();
        } else if (line.StartsWith("Installed Size ")) {
          currentApp.size = line.AfterFirst(':').Trim();
        } else if (line.StartsWith("Install Reason ")) {
          currentApp.installReason = line.AfterFirst(':').Trim();
        } else if (line.StartsWith("Depends On ")) {
          currentApp.dependsOn = line.AfterFirst(':').Trim();
        } else if (line.StartsWith("Install Date ")) {
          currentApp.installDate = line.AfterFirst(':').Trim();
        } else if (line.StartsWith("URL ")) {
          currentApp.url = line.AfterFirst(':').Trim();
        }
      }

      if (!currentApp.name.IsEmpty()) {
        apps.push_back(currentApp);
      }

      if (apps.empty()) {
        throw std::runtime_error("No applications found");
      }

      return apps;
    }

    void AdjustColumnWidths() {
      int totalWidth = m_listCtrl->GetClientSize().GetWidth();
      int numColumns = m_listCtrl->GetColumnCount();
      if (numColumns <= 0) {
        wxLogWarning("No columns found in list control");
        return;
      }

      int avgWidth = totalWidth / numColumns;
      for (int i = 0; i < numColumns; ++i) {
        m_listCtrl->SetColumnWidth(i, avgWidth);
      }
    }

    void OnSize(wxSizeEvent &event) {
      AdjustColumnWidths();
      event.Skip();
    }
  };

  ApplicationsPanel *applications = new ApplicationsPanel(appsInfoPage);
  appInfoSizer->Add(applications, 1, wxEXPAND | wxALL, 5);
  appsInfoPage->SetSizer(appInfoSizer);
  appsInfoPage->Layout();

  /************ END: END OF PAGE 4 ***************************/

  // Add pages to treebook
  treebook->AddPage(systeminfoPage, "System Information");   // page1
  treebook->AddPage(resourcesInfoPage, "Processes/Threads"); // page2
  treebook->AddPage(miscInfoPage, "Miscellaneous Info");     // page3
  treebook->AddPage(appsInfoPage, "Installed Applications"); // page4

  // use a sizer to layout the treebook with the frame
  wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
  sizer->Add(treebook, 1, wxEXPAND);

  frame->Show(true);
  return true;
}

MyFrame::MyFrame() : wxFrame(nullptr, wxID_ANY, "Sytem Information") {

  wxMenu *menuFile = new wxMenu;
  menuFile->Append(ID_Hello, "&Hello...\tCtrl-H",
                   "Help string shown in status bar for this menu item");
  menuFile->AppendSeparator();
  menuFile->Append(wxID_EXIT);

  wxMenu *menuHelp = new wxMenu;
  menuHelp->Append(wxID_ABOUT);

  wxMenuBar *menuBar = new wxMenuBar;
  menuBar->Append(menuFile, "&File");
  menuBar->Append(menuHelp, "&Help");

  SetMenuBar(menuBar);

  CreateStatusBar();
  SetStatusText("Built by Wanjare using C++ ");

  Bind(wxEVT_MENU, &MyFrame::OnHello, this, ID_Hello);
  Bind(wxEVT_MENU, &MyFrame::OnAbout, this, wxID_ABOUT);
  Bind(wxEVT_MENU, &MyFrame::OnExit, this, wxID_EXIT);

  // Set App Icon
  // wxString iconPath = wxFileName::GetCwd() + wxFILE_SEP_PATH + "favicon.ico";
  wxString iconPath = "./favicon.ico";
  if (wxFileExists(iconPath)) {
    wxIcon icon;
    if (icon.LoadFile(iconPath, wxBITMAP_TYPE_ICO)) {
      SetIcon(icon);
    } else {
      wxLogWarning("Failed to load icon from file in: %s", iconPath);
    }
  } else {
    wxLogWarning("Icon file not found in: %s", iconPath);
  }
};

void MyFrame::OnExit(wxCommandEvent &event) { Close(true); };

void MyFrame::OnAbout(wxCommandEvent &event) {
  wxMessageBox("Developer: Wanjare Samuel\nDate: Friday 7th May 2024\nWritten "
               "in C++\nView Your system Info hassle free",
               "About System Info", wxOK | wxICON_INFORMATION);
};

void MyFrame::OnHello(wxCommandEvent &event) {
  wxLogMessage("This application is running under %s.\n \
      Toolkit Type: wxGTK3\n \
      Toolkit Used: %s.",
               wxPlatformInfo::Get().GetOperatingSystemIdName(),
               wxVERSION_STRING);
}

/*
 * NOTE: I'm writing C++ 23 in 2024 because I'm crazy NO! NO, It's because I'm
 * badass shitty s/w engineer! I hope you enjoy the monolith. Check!
 */

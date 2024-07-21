/*
 *
 * @Author: Wanjare Samuel
 * @APPNAME: Sysinfo
 * PROGRAM: System Information Viewer
 * DESCRIPTION: View Information About your system with ease
 * LIBRARY: wxWidgets 3.2.5
 * FROM: 1st July 2024 Siaya, Kenya.
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
#include <wx/textfile.h>
#include <wx/tokenzr.h>
#include <wx/stc/stc.h>

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
  wxPanel *cpuInfoPage = new wxPanel(treebook, wxID_ANY);

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
  wxStaticText* cpuUsageTitle = new wxStaticText(systeminfoPage, wxID_ANY, "CPU USAGE",
      wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_HORIZONTAL);
  topRowLeftChild->Add(cpuUsageTitle, 0, wxEXPAND | wxALL, 5);

  // topRowLeftChild->Add(new wxStaticText(systeminfoPage, wxID_ANY, "CPU USAGE"),
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
          // info.command = wxJoin(fields.begin() + 10, fields.end(), ' ');

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

          // wxButton *killButton =
          //     new wxButton(m_listCtrl, wxID_ANY, "Kill", wxDefaultPosition,
          //                  wxDefaultSize, wxBU_EXACTFIT);
          // killButton->Bind(wxEVT_BUTTON, &ResourcesInfoPage::OnKillProcess,
          //                  this, wxID_ANY, wxID_ANY,
          //                  new wxVariant(static_cast<wxLongLong>(info.pid)));
          // m_listCtrl->SetItemPtrData(itemIndex,
          //                            reinterpret_cast<wxUIntPtr>(killButton));
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

  //-------------- Create the six sizers --------------------
  // Create main sizer
  wxBoxSizer* p3MainSizer = new wxBoxSizer(wxVERTICAL);

  // Create top row sizer
  wxFlexGridSizer* p3TopRowSizer = new wxFlexGridSizer(1, 3, 5, 5);
  p3TopRowSizer->AddGrowableCol(0, 1);
  p3TopRowSizer->AddGrowableCol(1, 1);
  p3TopRowSizer->AddGrowableCol(2, 1);

  // Create bottom row sizer
  wxFlexGridSizer* p3BottomRowSizer = new wxFlexGridSizer(1, 3, 5, 5);
  p3BottomRowSizer->AddGrowableCol(0, 1);
  p3BottomRowSizer->AddGrowableCol(1, 1);
  p3BottomRowSizer->AddGrowableCol(2, 1);

  // Create styled font for headers
  wxFont headerFont = wxFont(wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
      wxFONTWEIGHT_BOLD);

  // Create and add content to top row
  wxStaticText* topLeft = new wxStaticText(miscInfoPage, wxID_ANY, "Top Left", wxDefaultPosition,
      wxDefaultSize, wxALIGN_CENTER_HORIZONTAL);
  wxStaticText* topCenter = new wxStaticText(miscInfoPage, wxID_ANY, "Top Center", wxDefaultPosition,
      wxDefaultSize, wxALIGN_CENTER_HORIZONTAL);
  wxStaticText* topRight = new wxStaticText(miscInfoPage, wxID_ANY, "Top Right", wxDefaultPosition,
      wxDefaultSize, wxALIGN_CENTER_HORIZONTAL);

  topLeft->SetFont(headerFont);
  topCenter->SetFont(headerFont);
  topRight->SetFont(headerFont);

  p3TopRowSizer->Add(topLeft, 1, wxEXPAND | wxALL, 5);
  p3TopRowSizer->Add(topCenter, 1, wxEXPAND | wxALL, 5);
  p3TopRowSizer->Add(topRight, 1, wxEXPAND | wxALL, 5);

  // Create and add content to bottom row
  wxStaticText* bottomLeft = new wxStaticText(miscInfoPage, wxID_ANY, "Bottom Left",
      wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_HORIZONTAL);
  wxStaticText* bottomCenter = new wxStaticText(miscInfoPage, wxID_ANY, "Bottom Center",
      wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_HORIZONTAL);
  wxStaticText* bottomRight = new wxStaticText(miscInfoPage, wxID_ANY, "Bottom Right",
      wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_HORIZONTAL);

  bottomLeft->SetFont(headerFont);
  bottomCenter->SetFont(headerFont);
  bottomRight->SetFont(headerFont);

  p3BottomRowSizer->Add(bottomLeft, 1, wxEXPAND | wxALL, 5);
  p3BottomRowSizer->Add(bottomCenter, 1, wxEXPAND | wxALL, 5);
  p3BottomRowSizer->Add(bottomRight, 1, wxEXPAND | wxALL, 5);

  // Add row sizers to main sizer
  p3MainSizer->Add(topRowSizer, 1, wxEXPAND | wxALL, 10);
  p3MainSizer->Add(bottomRowSizer, 1, wxEXPAND | wxALL, 10);

  // Set the sizer for the panel
  p3MainSizer->Add(miscInfoPage, 1, wxEXPAND);
  //miscInfoPage->SetSizer(p3MainSizer);

  // Set minimum size for the frame
  //SetMinSize(wxSize(400, 300));

  /************ END: END OF PAGE 3 ***************************/

  /************* BEGIN: Populate page 4 values ***************/

  /************ END: END OF PAGE 4 ***************************/

  // Add some contents to each page
  wxStaticText *text4 =
      new wxStaticText(cpuInfoPage, wxID_ANY, "This is CPU fetails page");

  // Add pages to treebook
  treebook->AddPage(systeminfoPage, "System Information");    //page1
  treebook->AddPage(resourcesInfoPage, "Processes/Threads");  //page2
  treebook->AddPage(miscInfoPage, "Miscellaneous Info");      //page3
  treebook->AddPage(cpuInfoPage, "CPU Details");              //page4

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
  wxString iconPath = wxFileName::GetCwd() + wxFILE_SEP_PATH + "favicon.ico";
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

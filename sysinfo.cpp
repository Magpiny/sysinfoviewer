/*
 * System Info Viewer
 * Copyright (c) 2024 Wanjare Samuel
 *
 * This software is licensed under the MIT License.
 * See the LICENSE file in the project root for full license text.
 */

/*
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

#include <string>
#include <wx/app.h>
#include <wx/file.h>
#include <wx/filename.h>
#include <wx/icon.h>
#include <wx/listctrl.h>
#include <wx/menu.h>
#include <wx/platform.h>
#include <wx/platinfo.h>
#include <wx/process.h>
#include <wx/regex.h>
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

#include <fstream>
#include <sys/statvfs.h>
#include <wx/dcbuffer.h>

#include <arpa/inet.h>
#include <wx/dir.h>
#include <wx/log.h>
#include <wx/stc/stc.h>
#include <wx/textfile.h>
#include <wx/tokenzr.h>

#include <cstring>
#include <curl/curl.h>
#include <errno.h>
#include <exception>
#include <fcntl.h>
#include <filesystem>
#include <ifaddrs.h>
#include <linux/if_link.h>
#include <linux/if_packet.h>

#include <net/if.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>

#include <resolv.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <unistd.h>
#include <wx/event.h>
#include <wx/panel.h>
#include <wx/scrolwin.h>
#include <wx/statbmp.h>
#include <wx/thread.h>

// display libs
#include <wx/display.h>

// sound check
#include <alsa/asoundlib.h>

// DRM for display info
#include <xf86drm.h>
#include <xf86drmMode.h>

// app info
#include <cstdio>
#include <cstdlib>

#include <stdexcept>
#include <wx/wrapsizer.h>

#include <optional>
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
  SetAppDisplayName("System Monitor");
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

    // Get CPU Information
    void GetCPUInfo() {
      std::ifstream cpuinfo("/proc/cpuinfo");
      std::string line;
      int threadCount = 0;
      int coresPerPackage = 0;
      double speed = 0.0;

      while (std::getline(cpuinfo, line)) {
        if (line.substr(0, 9) == "processor") {
          threadCount++;
        } else if (line.substr(0, 9) == "cpu cores") {
          size_t pos = line.find(':');
          if (pos != std::string::npos) {
            coresPerPackage = std::stoi(line.substr(pos + 1));
          }
        } else if (line.substr(0, 7) == "cpu MHz") {
          size_t pos = line.find(':');
          if (pos != std::string::npos) {
            speed = std::stod(line.substr(pos + 1));
          }
        }
      }

      m_cpuThreads = threadCount;
      // coresPerPackage is per physical CPU; for single-socket this equals
      // total cores
      m_cpuCores = (coresPerPackage > 0) ? coresPerPackage : threadCount;
      m_cpuCount =
          threadCount; // keep m_cpuCount as threads for backward compat
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
      wxString cpuInfo =
          wxString::Format("Cores: %d | Threads: %d | Speed: %.0f MHz",
                           m_cpuCores, m_cpuThreads, m_cpuSpeed);
      gc->SetFont(wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
                         wxFONTWEIGHT_NORMAL),
                  wxColor(80, 80, 80));
      gc->GetTextExtent(cpuInfo, &textWidth, &textHeight);
      gc->DrawText(cpuInfo, centerX - textWidth / 2, height - textHeight - 10);
    }

    wxTimer m_timer;
    double m_cpuUsage;
    int m_cpuCount;   // logical processors / threads
    int m_cpuCores;   // physical cores
    int m_cpuThreads; // same as m_cpuCount, explicit alias
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

      // Add labels: percentage at the center
      wxString percentageText =
          wxString::Format("%.1f%%", (double)m_usedSpace / m_totalSpace * 100);
      gc->SetFont(wxFont(20, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
                         wxFONTWEIGHT_BOLD),
                  wxColor(80, 80, 80));
      double textWidth, textHeight;
      gc->GetTextExtent(percentageText, &textWidth, &textHeight);
      gc->DrawText(percentageText, centerX - textWidth / 2,
                   centerY - textHeight / 2);

      // Horizontal labels below the chart
      struct LabelEntry {
        wxString text;
        wxColor color;
      };
      LabelEntry labels[] = {
          {"Total: " + FormatSize(m_totalSpace), wxColor(80, 80, 80)},
          {"Used: " + FormatSize(m_usedSpace), wxColor(200, 60, 60)},
          {"Free: " + FormatSize(m_freeSpace), wxColor(80, 130, 80)},
      };

      gc->SetFont(wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
                         wxFONTWEIGHT_BOLD),
                  wxColor(80, 80, 80));

      // Measure all three so we can centre the group
      double totalLabelWidth = 0, labelH = 0;
      const double padding = 20; // gap between labels
      double widths[3] = {};
      for (int i = 0; i < 3; i++) {
        gc->SetFont(wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
                           wxFONTWEIGHT_BOLD),
                    labels[i].color);
        double tw, th;
        gc->GetTextExtent(labels[i].text, &tw, &th);
        widths[i] = tw;
        totalLabelWidth += tw;
        if (th > labelH)
          labelH = th;
      }
      totalLabelWidth += padding * 2; // two gaps between three labels

      double labelY = centerY + radius + 12;
      double startX = centerX - totalLabelWidth / 2;

      for (int i = 0; i < 3; i++) {
        gc->SetFont(wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
                           wxFONTWEIGHT_BOLD),
                    labels[i].color);
        gc->DrawText(labels[i].text, startX, labelY);
        startX += widths[i] + padding;
      }
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
  wxArrayString uname_output;
  wxString arch_str = "Unknown";
  if (wxExecute("uname -m", uname_output) == 0 && !uname_output.IsEmpty()) {
    if (uname_output[0] == "x86_64") {
      arch_str = "64-bit";
    } else if (uname_output[0].Contains("386") ||
               uname_output[0].Contains("i686")) {
      arch_str = "32-bit";
    } else {
      arch_str = uname_output[0];
    }
  }
  wxStaticText *cpuArchitecture =
      new wxStaticText(systeminfoPage, wxID_ANY, arch_str);

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
      m_timer->Start(5000);
      FindBattery();
      UpdateBatteryInfo();
      SetMinSize(wxSize(500, 400));
    }

    ~BatteryInfoPanel() {
      if (m_timer) {
        m_timer->Stop();
        delete m_timer;
      }
    }

  private:
    wxString m_batteryPath;
    wxTimer *m_timer = nullptr;

    // --- parsed fields ---
    int m_percentage = 0;
    double m_energyNow = 0.0;        // Wh
    double m_energyFull = 0.0;       // Wh
    double m_energyFullDesign = 0.0; // Wh
    double m_energyRate = 0.0;       // W  (power draw / charge rate)
    double m_voltage = 0.0;          // V
    double m_voltageMinDesign = 0.0; // V
    int m_cycleCnt = 0;
    int m_healthPct = 100;
    wxString m_state = "Unknown";
    wxString m_technology = "Unknown";
    wxString m_capacityLevel = "Unknown";
    wxString m_warningLevel = "none";
    wxString m_vendor = "Unknown";
    wxString m_model = "Unknown";
    wxString m_serial = "Unknown";
    wxString m_timeRemaining = "N/A";
    bool m_hasBattery = false;

    // ------------------------------------------------------------------ //
    wxString ReadFile(const wxString &path) {
      wxTextFile f(path);
      if (f.Open()) {
        wxString s = f.GetFirstLine().Trim();
        f.Close();
        return s;
      }
      return wxEmptyString;
    }

    // Convert µWh  →  Wh   (sysfs stores energy in µWh)
    double ToWh(const wxString &raw) {
      double v = 0.0;
      raw.ToDouble(&v);
      return v / 1e6;
    }

    // Convert µV  →  V
    double ToV(const wxString &raw) {
      double v = 0.0;
      raw.ToDouble(&v);
      return v / 1e6;
    }

    // Convert µW  →  W
    double ToW(const wxString &raw) {
      double v = 0.0;
      raw.ToDouble(&v);
      return v / 1e6;
    }

    // ------------------------------------------------------------------ //
    void FindBattery() {
      m_hasBattery = false;
      const wxString base = "/sys/class/power_supply/";
      wxArrayString candidates;
      candidates.Add("BAT0");
      candidates.Add("BAT1");
      candidates.Add("battery");
      candidates.Add("BATT");

      for (const auto &name : candidates) {
        wxString path = base + name + "/";
        if (wxDir::Exists(path) &&
            ReadFile(path + "type").IsSameAs("Battery", false)) {
          m_batteryPath = path;
          m_hasBattery = true;
          return;
        }
      }
    }

    // ------------------------------------------------------------------ //
    void UpdateBatteryInfo() {
      if (!m_hasBattery)
        return;
      const wxString &p = m_batteryPath;

      // --- state & percentage ---
      m_state = ReadFile(p + "status");
      m_technology = ReadFile(p + "technology");
      m_capacityLevel = ReadFile(p + "capacity_level");
      m_warningLevel = ReadFile(p + "alarm"); // not always present
      ReadFile(p + "capacity").ToInt(&m_percentage);

      // --- energy (µWh nodes preferred; fall back to charge_* µAh * voltage)
      // ---
      wxString eNow = ReadFile(p + "energy_now");
      wxString eFull = ReadFile(p + "energy_full");
      wxString eDesign = ReadFile(p + "energy_full_design");

      if (!eNow.IsEmpty()) {
        m_energyNow = ToWh(eNow);
        m_energyFull = ToWh(eFull);
        m_energyFullDesign = ToWh(eDesign);
      } else {
        // charge-based sysfs layout (µAh): convert using voltage
        double vNow = ToV(ReadFile(p + "voltage_now"));
        if (vNow == 0.0)
          vNow = 11.1; // safe fallback
        auto uAhToWh = [&](const wxString &raw) -> double {
          double ua = 0.0;
          raw.ToDouble(&ua);
          return (ua / 1e6) * vNow;
        };
        m_energyNow = uAhToWh(ReadFile(p + "charge_now"));
        m_energyFull = uAhToWh(ReadFile(p + "charge_full"));
        m_energyFullDesign = uAhToWh(ReadFile(p + "charge_full_design"));
      }

      // --- power rate ---
      wxString pNow = ReadFile(p + "power_now");
      if (!pNow.IsEmpty()) {
        m_energyRate = ToW(pNow);
      } else {
        // current_now (µA) * voltage_now (µV) → W
        double iua = 0.0, vuv = 0.0;
        ReadFile(p + "current_now").ToDouble(&iua);
        ReadFile(p + "voltage_now").ToDouble(&vuv);
        m_energyRate = (iua / 1e6) * (vuv / 1e6);
      }

      // --- voltage ---
      m_voltage = ToV(ReadFile(p + "voltage_now"));
      m_voltageMinDesign = ToV(ReadFile(p + "voltage_min_design"));

      // --- cycle count ---
      ReadFile(p + "cycle_count").ToInt(&m_cycleCnt);

      // --- health % ---
      if (m_energyFullDesign > 0.0)
        m_healthPct =
            static_cast<int>(m_energyFull / m_energyFullDesign * 100.0);

      // --- time remaining ---
      m_timeRemaining = "N/A";
      if (m_energyRate > 0.01) {
        double hours = (m_state.IsSameAs("Charging", false))
                           ? (m_energyFull - m_energyNow) / m_energyRate
                           : m_energyNow / m_energyRate;
        int h = static_cast<int>(hours);
        int m = static_cast<int>((hours - h) * 60);
        m_timeRemaining = wxString::Format("%d h %02d min", h, m);
      }

      // --- vendor / model / serial via upower (best-effort) ---
      FetchUpowerMeta();
    }

    // ------------------------------------------------------------------ //
    // Run upower once to get vendor/model/serial — fields not in sysfs
    void FetchUpowerMeta() {
      wxArrayString out;
      // Find the upower device path first
      wxArrayString devs;
      if (wxExecute("upower -e", devs, wxEXEC_SYNC) != 0 || devs.IsEmpty())
        return;

      wxString batDev;
      for (const auto &d : devs)
        if (d.Contains("BAT") || d.Contains("battery")) {
          batDev = d;
          break;
        }
      if (batDev.IsEmpty())
        return;

      if (wxExecute("upower -i " + batDev, out, wxEXEC_SYNC) != 0)
        return;

      auto extract = [&](const wxString &key) -> wxString {
        for (const auto &line : out) {
          int idx = line.Find(key);
          if (idx != wxNOT_FOUND) {
            wxString val = line.Mid(idx + key.Len()).Trim(false).Trim();
            return val;
          }
        }
        return "Unknown";
      };

      m_vendor = extract("vendor:");
      m_model = extract("model:");
      m_serial = extract("serial:");

      // upower's time string is more accurate — prefer it
      wxString upTime = extract("time to empty:");
      if (upTime.IsEmpty() || upTime == "Unknown")
        upTime = extract("time to full:");
      if (!upTime.IsEmpty() && upTime != "Unknown")
        m_timeRemaining = upTime;
    }

    // ------------------------------------------------------------------ //
    wxColor BatteryColor() const {
      if (m_percentage > 85)
        return wxColor(40, 180, 40);
      if (m_percentage > 40)
        return wxColor(255, 165, 0);
      return wxColor(210, 40, 40);
    }

    // ------------------------------------------------------------------ //
    void OnPaint(wxPaintEvent &) {
      wxAutoBufferedPaintDC dc(this);
      dc.Clear();
      wxGraphicsContext *gc = wxGraphicsContext::Create(dc);
      if (!gc)
        return;

      wxSize sz = GetClientSize();
      int W = sz.GetWidth();
      int H = sz.GetHeight();

      // ── battery bar (left column) ──────────────────────────────────
      const int barW = 40;
      const int barH = H - 120;
      const int barX = 30;
      const int barY = 50;
      const int tipH = 10;
      const int tipW = 16;

      // tip nub
      gc->SetBrush(wxBrush(wxColor(160, 160, 160)));
      gc->SetPen(*wxTRANSPARENT_PEN);
      gc->DrawRectangle(barX + (barW - tipW) / 2, barY - tipH, tipW, tipH);

      // outline
      gc->SetBrush(*wxWHITE_BRUSH);
      gc->SetPen(wxPen(wxColor(80, 80, 80), 2));
      gc->DrawRoundedRectangle(barX, barY, barW, barH, 4);

      // fill
      int fillH = static_cast<int>(barH * m_percentage / 100.0);
      gc->SetBrush(wxBrush(BatteryColor()));
      gc->SetPen(*wxTRANSPARENT_PEN);
      gc->DrawRoundedRectangle(barX, barY + barH - fillH, barW, fillH, 4);

      // percentage text centred in bar
      gc->SetFont(wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
                         wxFONTWEIGHT_BOLD),
                  *wxBLACK);
      wxString pctStr = wxString::Format("%d%%", m_percentage);
      double tw, th;
      gc->GetTextExtent(pctStr, &tw, &th);
      gc->DrawText(pctStr, barX + (barW - tw) / 2, barY + barH + 6);

      // ── info table (right column) ──────────────────────────────────
      const int col1X = barX + barW + 24;
      const int col2X = col1X + 170;
      int rowY = barY;
      const int rowH = 22;

      auto row = [&](const wxString &label, const wxString &value,
                     const wxColor &valCol = wxColor(139,115,85)) {
        gc->SetFont(wxFont(9, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
                           wxFONTWEIGHT_BOLD),
                    wxColor(90, 90, 90));
        gc->DrawText(label, col1X, rowY);
        gc->SetFont(wxFont(9, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
                           wxFONTWEIGHT_NORMAL),
                    valCol);
        gc->DrawText(value, col2X, rowY);
        rowY += rowH;
      };

      // section heading
      gc->SetFont(wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
                         wxFONTWEIGHT_BOLD),
                  wxColor(101,67,33));
      gc->DrawText("BATTERY — " + m_batteryPath, col1X, rowY - rowH);
      rowY += 4;

      wxColor stateCol =
          m_state.IsSameAs("Charging", false)      ? wxColor(0, 150, 0)
          : m_state.IsSameAs("Discharging", false) ? wxColor(180, 80, 0)
                                                   : wxColor(60, 60, 60);

      row("Vendor", m_vendor);
      row("Model", m_model);
      row("Serial", m_serial);
      row("State", m_state, stateCol);
      row("Percentage", wxString::Format("%d %%", m_percentage));
      row("Energy now", wxString::Format("%.3f Wh", m_energyNow));
      row("Energy full", wxString::Format("%.3f Wh", m_energyFull));
      row("Energy full design",
          wxString::Format("%.3f Wh", m_energyFullDesign));
      row("Energy rate", wxString::Format("%.3f W", m_energyRate));
      row("Voltage", wxString::Format("%.3f V", m_voltage));
      row("Voltage min design", wxString::Format("%.3f V", m_voltageMinDesign));
      row("Capacity", wxString::Format("%d %%", m_healthPct));
      row("Capacity level", m_capacityLevel);
      row("Charge cycles", wxString::Format("%d", m_cycleCnt));
      row("Technology", m_technology);
      row("Time remaining", m_timeRemaining,
          m_state.IsSameAs("Charging", false) ? wxColor(0, 130, 0)
                                              : wxColor(160, 60, 0));

      delete gc;
    }

    // ------------------------------------------------------------------ //
    void OnTimer(wxTimerEvent &) {
      UpdateBatteryInfo();
      Refresh();
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
          wxArrayString commandParts;
          std::copy(fields.begin() + 10, fields.end(),
                    std::back_inserter(commandParts));
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
  wxStaticBoxSizer *displaySizer =
      new wxStaticBoxSizer(wxVERTICAL, displayInfoPane, "DISPLAY INFORMATION");

  class DisplayInfoPanel : public wxPanel {
  public:
    DisplayInfoPanel(wxWindow *parent) : wxPanel(parent) {
      wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
      SetSizer(mainSizer);

      CalculateBaseFontSize();
      CollectSystemInfo();
      CreateInfoDisplay();

      Bind(wxEVT_SIZE, &DisplayInfoPanel::OnSize, this);
    }

  private:
    struct SystemInfo {
      std::optional<wxString> desktopEnv;
      unsigned displayCount = 0;
      std::optional<wxString> resolution;
      std::optional<wxString> refreshRate;
      std::optional<int> colorDepth;
      std::optional<int> brightness;
      std::optional<wxString> manufacturer;
      std::optional<wxString> model;
      std::optional<wxString> screenSizeInches;
    } m_systemInfo;

    int m_baseFontSize = 10;
    std::vector<wxStaticText *> m_infoLabels;

    void CalculateBaseFontSize() {
      wxDisplay display(0u);
      wxSize ppi = display.GetPPI();
      m_baseFontSize = wxMax(10, wxMin(ppi.GetWidth() / 10, 16));
      if (ppi.GetWidth() > 120) {
        m_baseFontSize += 2;
      }
    }

    void OnSize(wxSizeEvent &event) {
      UpdateFontSizes();
      event.Skip();
    }

    void UpdateFontSizes() {
      int windowWidth = GetSize().GetWidth();
      int dynamicSize = wxMax(m_baseFontSize - 2,
                              wxMin(m_baseFontSize + 2, windowWidth / 50));
      wxFont font = GetFont();
      font.SetPointSize(dynamicSize);
      for (auto *label : m_infoLabels) {
        label->SetFont(font);
      }
      Layout();
    }

    void CollectSystemInfo() {
      m_systemInfo.desktopEnv = GetDesktopEnvironment();
      try {
        m_systemInfo.displayCount = wxDisplay::GetCount();
      } catch (...) {
        m_systemInfo.displayCount = 0;
      }

      wxArrayString xrandr_output;
      if (wxExecute("xrandr", xrandr_output) == 0) {
        for (const wxString &line : xrandr_output) {
          if (line.Contains(" connected primary")) {
            wxRegEx re(" (\\d+x\\d+) ");
            if (re.Matches(line)) {
              m_systemInfo.resolution = re.GetMatch(line, 1);
            }
            wxRegEx size_re(" (\\d+)mm x (\\d+)mm");
            if (size_re.Matches(line)) {
              double width_mm, height_mm;
              size_re.GetMatch(line, 1).ToDouble(&width_mm);
              size_re.GetMatch(line, 2).ToDouble(&height_mm);
              if (width_mm > 0 && height_mm > 0) {
                double diagonal_inches =
                    std::sqrt(width_mm * width_mm + height_mm * height_mm) /
                    25.4;
                m_systemInfo.screenSizeInches =
                    wxString::Format("%.1f\"", diagonal_inches);
              }
            }
            break;
          }
        }
      }

      if (!m_systemInfo.resolution && m_systemInfo.displayCount > 0) {
        wxDisplay display(0u);
        wxRect geom = display.GetGeometry();
        m_systemInfo.resolution =
            wxString::Format("%dx%d", geom.width, geom.height);
      }

      if (m_systemInfo.displayCount > 0) {
        try {
          wxDisplay display(0u);
          wxVideoMode mode = display.GetCurrentMode();
          m_systemInfo.colorDepth = mode.GetDepth();
        } catch (...) {
        }
      }

      m_systemInfo.refreshRate = GetRefreshRate();
      m_systemInfo.brightness = GetBrightness();
      auto edidInfo = GetEdidInfo();
      if (edidInfo) {
        m_systemInfo.manufacturer = edidInfo->first;
        m_systemInfo.model = edidInfo->second;
      }
    }

    std::optional<wxString> GetDesktopEnvironment() {
      const char *de = std::getenv("XDG_CURRENT_DESKTOP");
      if (de)
        return wxString(de);
      wxArrayString output;
      if (wxExecute("cat /etc/*-release | grep PRETTY_NAME", output) == 0 &&
          !output.IsEmpty()) {
        wxString prettyName = output[0].AfterFirst('=').Trim().Trim(false);
        prettyName = prettyName.Mid(1, prettyName.Length() - 2);
        return prettyName;
      }
      return std::nullopt;
    }

    std::optional<wxString> GetRefreshRate() {
      wxArrayString output;
      if (wxExecute("xrandr", output) == 0) {
        for (const wxString &line : output) {
          if (line.Contains("*")) {
            wxStringTokenizer tokenizer(line);
            while (tokenizer.HasMoreTokens()) {
              wxString token = tokenizer.GetNextToken();
              if (token.Contains('*')) {
                token.Replace("*", "");
                token.Replace("+", "");
                double rate;
                if (token.ToDouble(&rate)) {
                  return wxString::Format("%.2f", rate);
                }
              }
            }
          }
        }
      }
      return std::nullopt;
    }

    std::optional<int> GetBrightness() {
      namespace fs = std::filesystem;
      const fs::path backlight_path("/sys/class/backlight");
      if (fs::exists(backlight_path)) {
        for (const auto &entry : fs::directory_iterator(backlight_path)) {
          if (entry.is_directory()) {
            fs::path brightness_file = entry.path() / "brightness";
            fs::path max_brightness_file = entry.path() / "max_brightness";
            if (fs::exists(brightness_file) &&
                fs::exists(max_brightness_file)) {
              std::ifstream current_fs(brightness_file);
              std::ifstream max_fs(max_brightness_file);
              long current_val, max_val;
              if (current_fs >> current_val && max_fs >> max_val &&
                  max_val > 0) {
                return static_cast<int>(
                    (static_cast<double>(current_val) / max_val) * 100);
              }
            }
          }
        }
      }
      return std::nullopt;
    }

    bool GetEdidFromDrm(std::vector<unsigned char> &edid_data) {
      for (int i = 0; i < 16; ++i) {
        std::string card_path = "/dev/dri/card" + std::to_string(i);
        int fd = open(card_path.c_str(), O_RDONLY);
        if (fd < 0)
          continue;
        drmModeRes *res = drmModeGetResources(fd);
        if (!res) {
          close(fd);
          continue;
        }
        bool found = false;
        for (int j = 0; j < res->count_connectors; ++j) {
          drmModeConnector *conn = drmModeGetConnector(fd, res->connectors[j]);
          if (conn && conn->connection == DRM_MODE_CONNECTED) {
            for (int k = 0; k < conn->count_props; ++k) {
              drmModePropertyRes *prop = drmModeGetProperty(fd, conn->props[k]);
              if (prop && strcmp(prop->name, "EDID") == 0) {
                drmModePropertyBlobRes *blob =
                    drmModeGetPropertyBlob(fd, conn->prop_values[k]);
                if (blob && blob->length > 0) {
                  edid_data.assign((unsigned char *)blob->data,
                                   (unsigned char *)blob->data + blob->length);
                  found = true;
                  drmModeFreePropertyBlob(blob);
                }
                drmModeFreeProperty(prop);
                if (found)
                  break;
              }
              if (prop)
                drmModeFreeProperty(prop);
            }
          }
          drmModeFreeConnector(conn);
          if (found)
            break;
        }
        drmModeFreeResources(res);
        close(fd);
        if (found)
          return true;
      }
      return false;
    }

    std::optional<std::pair<wxString, wxString>> GetEdidInfo() {
      std::vector<unsigned char> edid;
      if (GetEdidFromDrm(edid) && edid.size() >= 128) {
        wxString manufacturer = wxString::Format(
            "%c%c%c", ((edid[8] & 0x7C) >> 2) + 'A' - 1,
            (((edid[8] & 0x03) << 3) | ((edid[9] & 0xE0) >> 5)) + 'A' - 1,
            (edid[9] & 0x1F) + 'A' - 1);
        wxString model;
        for (int i = 54; i < 126; i += 18) {
          if (edid[i] == 0x00 && edid[i + 1] == 0x00 && edid[i + 2] == 0x00) {
            int desc_type = edid[i + 3];
            if (desc_type == 0xFC || desc_type == 0xFE) {
              wxString str;
              for (int j = 5; j < 18; ++j) {
                if (edid[i + j] == 0x0A)
                  break;
                if (edid[i + j] >= 32 && edid[i + j] <= 126) {
                  str += static_cast<char>(edid[i + j]);
                }
              }
              str = str.Trim();
              if (!str.empty()) {
                model = str;
                break;
              }
            }
          }
        }
        return std::make_pair(manufacturer, model);
      }
      return std::nullopt;
    }

    void CreateInfoDisplay() {
      wxSizer *sizer = GetSizer();
      wxFont boldFont = GetFont();
      boldFont.MakeBold();
      boldFont.SetPointSize(m_baseFontSize);

      auto AddInfoLine = [&](const wxString &text) {
        wxStaticText *label = new wxStaticText(this, wxID_ANY, text);
        label->SetFont(boldFont);
        sizer->Add(label, 0, wxALL, 5);
        m_infoLabels.push_back(label);
      };

      if (m_systemInfo.desktopEnv) {
        AddInfoLine("DE: " + *m_systemInfo.desktopEnv);
      }
      AddInfoLine(wxString::Format("Displays: %u", m_systemInfo.displayCount));
      if (m_systemInfo.resolution) {
        AddInfoLine("Resolution: " + *m_systemInfo.resolution);
      }
      if (m_systemInfo.screenSizeInches) {
        AddInfoLine("Screen Size: " + *m_systemInfo.screenSizeInches);
      }
      if (m_systemInfo.refreshRate) {
        AddInfoLine(
            wxString::Format("Refreshrate: %sHz", *m_systemInfo.refreshRate));
      }
      if (m_systemInfo.colorDepth) {
        AddInfoLine(
            wxString::Format("Color Depth: %d bit", *m_systemInfo.colorDepth));
      }
      if (m_systemInfo.brightness) {
        AddInfoLine(
            wxString::Format("Brightness: %d%%", *m_systemInfo.brightness));
      }
      if (m_systemInfo.manufacturer) {
        AddInfoLine("Display Manufacturer: " + *m_systemInfo.manufacturer);
      }
      // if (m_systemInfo.model) {
      //     AddInfoLine("Display Model: " + *m_systemInfo.model);
      // }

      UpdateFontSizes();
    }
  };

  // Add DISPLAY INFORMATION to the top center of the page 3
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

  //----------------------- CPU INFORMATION ---------------------------------
  wxStaticBoxSizer *cpuInfoSizer =
      new wxStaticBoxSizer(wxVERTICAL, cpuInfoPane, "CPU & GPU INFORMATION");

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

        // Section divider row
        if (value.IsEmpty()) {
          wxStaticText *divider =
              new wxStaticText(this, wxID_ANY, key, wxDefaultPosition,
                               wxDefaultSize, wxALIGN_CENTRE_HORIZONTAL);
          divider->SetFont(divider->GetFont().Bold());
          divider->SetForegroundColour(wxColour(50, 50, 180));
          m_mainSizer->Add(divider, 0, wxEXPAND | wxTOP | wxBOTTOM, 8);
          continue;
        }

        // Normal key/value row
        wxBoxSizer *rowSizer = new wxBoxSizer(wxHORIZONTAL);
        wxStaticText *keyLabel = new wxStaticText(this, wxID_ANY, key + ":");
        keyLabel->SetFont(keyLabel->GetFont().Bold());
        wxStaticText *valueLabel = new wxStaticText(this, wxID_ANY, value);

        rowSizer->Add(keyLabel, 0, wxALL, 5);
        rowSizer->Add(valueLabel, 1, wxALL, 5);
        m_mainSizer->Add(rowSizer, 0, wxEXPAND | wxALL, 3);
      }
    }

    std::vector<std::pair<wxString, wxString>> GetCPUInfo() {
          std::vector<std::pair<wxString, wxString>> info;
          std::ifstream cpuinfo("/proc/cpuinfo");
          std::string line;
          std::string model_name, vendor_id;
          int physicalCores = 0;
          m_numCores = 0;  // reset before counting

          // Single pass — collect everything from /proc/cpuinfo
          while (std::getline(cpuinfo, line)) {
            std::istringstream iss(line);
            std::string key, value;
            if (std::getline(iss, key, ':') && std::getline(iss, value)) {
              key   = Trim(key);
              value = Trim(value);
              if (key == "processor")
                m_numCores++;
              if (key == "model name" && model_name.empty())
                model_name = value;
              if (key == "vendor_id" && vendor_id.empty())
                vendor_id = value;
              if (key == "cpu cores" && physicalCores == 0)
                physicalCores = std::stoi(value);
            }
          }
          if (physicalCores == 0) physicalCores = m_numCores;

          // Architecture
          wxArrayString uname_output;
          wxString arch_str = "Unknown";
          if (wxExecute("uname -m", uname_output, wxEXEC_SYNC) == 0 &&
              !uname_output.IsEmpty()) {
            if      (uname_output[0] == "x86_64")            arch_str = "64-bit";
            else if (uname_output[0].Contains("386") ||
                     uname_output[0].Contains("i686"))        arch_str = "32-bit";
            else                                              arch_str = uname_output[0];
          }

          // ── CPU section ───────────────────────────────────────────────────
          info.emplace_back("CPU Manufacturer",   vendor_id);
          info.emplace_back("CPU Model",          model_name);
          info.emplace_back("CPU Architecture",   arch_str);
          info.emplace_back("CPU Physical Cores", std::to_string(physicalCores));
          info.emplace_back("CPU Logical Threads",std::to_string(m_numCores));

          // ── GPU section ───────────────────────────────────────────────────
          wxString gpuVendor = "Unknown", gpuName = "Unknown",
                   gpuDriver = "Unknown", gpuVRAM = "Unknown";

          // lspci — most reliable for name on any GPU
          wxArrayString lspci;
          if (wxExecute("lspci", lspci, wxEXEC_SYNC) == 0) {
            for (const auto &l : lspci) {
              if (l.Contains("VGA") || l.Contains("Display") ||
                  l.Contains("3D")  || l.Contains("GPU")) {
                gpuName = l.AfterFirst(':').AfterFirst(':').Trim(false);
                break;
              }
            }
          }

          // sysfs — vendor, driver, VRAM
          // Try card0 through card2 explicitly (wxDir glob can miss render nodes)
          for (const wxString &card : { wxString("card0"),
                                         wxString("card1"),
                                         wxString("card2") }) {
            wxString base = "/sys/class/drm/" + card + "/device/";
            if (!wxDirExists(base)) continue;

            // Vendor
            if (gpuVendor == "Unknown") {
              wxTextFile vf;
              if (vf.Open(base + "vendor")) {
                wxString vid = vf.GetFirstLine().Trim().Lower();
                vf.Close();
                if      (vid == "0x1002") gpuVendor = "AMD";
                else if (vid == "0x10de") gpuVendor = "NVIDIA";
                else if (vid == "0x8086") gpuVendor = "Intel";
                else                      gpuVendor = vid;
              }
            }

            // Driver — readlink on the driver symlink
            if (gpuDriver == "Unknown") {
              std::string dpath = std::string(base.mb_str()) + "driver";
              char buf[512] = {};
              ssize_t len = readlink(dpath.c_str(), buf, sizeof(buf) - 1);
              if (len > 0)
                gpuDriver = wxString::FromUTF8(buf).AfterLast('/');
            }

            // VRAM — amdgpu exposes this node
            if (gpuVRAM == "Unknown") {
              wxTextFile vramF;
              if (vramF.Open(base + "mem_info_vram_total")) {
                wxString raw = vramF.GetFirstLine().Trim();
                vramF.Close();
                unsigned long long bytes = 0;
                raw.ToULongLong(&bytes);
                if (bytes > 0)
                  gpuVRAM = wxString::Format("%.0f MB",
                                (double)bytes / (1024.0 * 1024.0));
              }
            }

            break; // found a valid card, stop
          }

          info.emplace_back("── GPU ──",      "");   // divider
          info.emplace_back("GPU Vendor",     gpuVendor);
          info.emplace_back("GPU Name",       gpuName);
          info.emplace_back("GPU Driver",     gpuDriver);
          info.emplace_back("GPU VRAM",       gpuVRAM);

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

MyFrame::MyFrame() : wxFrame(nullptr, wxID_ANY, "Sytem Information Monitor") {

  wxMenu *menuFile = new wxMenu;
  menuFile->Append(ID_Hello, "&Hello...\tCtrl-I",
                   "Show system information summary");
  menuFile->AppendSeparator();
  menuFile->Append(wxID_EXIT);

  wxMenu *menuHelp = new wxMenu;
  menuHelp->Append(wxID_ABOUT);

  wxMenuBar *menuBar = new wxMenuBar;
  menuBar->Append(menuFile, "&File");
  menuBar->Append(menuHelp, "&Help");

  SetMenuBar(menuBar);

  CreateStatusBar();
  SetStatusText("Built by Magpiny using C++ ");

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
  wxMessageBox(
      "Developer: Wanjare Samuel\nDate: Friday 7th May 2024\nWritten "
      "in C++\nView Your system Info hassle free \nApp Version: 0.3.1-beta",
      "About System Info", wxOK | wxICON_INFORMATION);
};

void MyFrame::OnHello(wxCommandEvent &event) {
  wxString osDesc = wxPlatformInfo::Get().GetOperatingSystemDescription();
  wxString userName = wxGetUserName();
  wxString currentTime = wxNow();
  wxString title = wxString::Format("Hello %s", userName);

  wxMessageBox(wxString::Format("User: %s\nDate: %s \nOS: %s", userName,
                                currentTime, osDesc),
               title, wxOK | wxICON_INFORMATION);
}

/*
 * NOTE: I'm writing C++ 23 in 2024 because I'm crazy NO! NO, It's because I'm
 * badass shitty s/w engineer! I hope you enjoy the monolith. Check!
 * Aol jowadu! Aol jowadu! Aol jowadu! Rao001 will be laid to rest tomorrow!
 * Next time I come here I'll have RAO001 timer
 */

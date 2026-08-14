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

#include <algorithm>
#include <climits>
#include <functional>
#include <map>
#include <optional>
#include <set>
#include <wx/artprov.h>
#include <wx/checkbox.h>
#include <wx/clipbrd.h>
#include <wx/dataobj.h>
#include <wx/dcmemory.h>
#include <wx/font.h>
#include <wx/imaglist.h>
#include <wx/srchctrl.h>
#include <wx/statline.h>
#include <wx/timer.h>

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
  wxScrolledWindow *miscInfoPage =
      new wxScrolledWindow(treebook, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                           wxVSCROLL | wxHSCROLL);
  miscInfoPage->SetScrollRate(10, 10);
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
                     const wxColor &valCol = wxColor(139, 115, 85)) {
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
                  wxColor(101, 67, 33));
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
  struct ProcessInfo {
    long pid = 0;
    wxString user;
    wxString name;
    double cpuVal = 0.0;
    wxString cpuFormatted;
    double ramPercentVal = 0.0;
    wxString ramPercentFormatted;
    long long rssKiB = 0;
    wxString ramFormatted;
    wxString time;
    wxString command;
    int iconIndex = -1;
  };

  class ProcessIconManager {
  public:
    static ProcessIconManager &Get() {
      static ProcessIconManager instance;
      return instance;
    }

    void Initialize(wxImageList *imageList) {
      m_imageList = imageList;
      InitCacheDir();
      ScanDesktopFiles();
    }

    int GetIconIndex(const wxString &procName, const wxString &cmdLine = "") {
      if (!m_imageList)
        return -1;

      wxString key = procName.Lower().Trim();
      if (key.IsEmpty())
        key = "unknown";

      // 1. Check in-memory index cache
      auto it = m_iconIndexMap.find(key);
      if (it != m_iconIndexMap.end()) {
        return it->second;
      }

      // 2. Check disk cache
      wxString cachedFilePath =
          m_cacheDir + wxFILE_SEP_PATH + SanitizeFileName(key) + ".png";
      if (wxFileExists(cachedFilePath)) {
        wxImage img;
        if (img.LoadFile(cachedFilePath, wxBITMAP_TYPE_PNG)) {
          if (img.GetWidth() != 24 || img.GetHeight() != 24) {
            img = img.Rescale(24, 24, wxIMAGE_QUALITY_HIGH);
          }
          wxBitmap bmp(img);
          int idx = m_imageList->Add(bmp);
          m_iconIndexMap[key] = idx;
          return idx;
        }
      }

      // 3. Try finding in system icon themes via desktop map or direct name
      wxString iconTarget;
      auto dIt = m_desktopMap.find(key);
      if (dIt != m_desktopMap.end()) {
        iconTarget = dIt->second;
      } else {
        iconTarget = key;
      }

      wxString systemIconPath = FindSystemIcon(iconTarget);
      if (systemIconPath.IsEmpty() && iconTarget != key) {
        systemIconPath = FindSystemIcon(key);
      }

      if (!systemIconPath.IsEmpty()) {
        wxImage img;
        if (img.LoadFile(systemIconPath)) {
          img = img.Rescale(24, 24, wxIMAGE_QUALITY_HIGH);
          // Save to disk cache for persistent fast boot
          img.SaveFile(cachedFilePath, wxBITMAP_TYPE_PNG);
          wxBitmap bmp(img);
          int idx = m_imageList->Add(bmp);
          m_iconIndexMap[key] = idx;
          return idx;
        }
      }

      // 4. Fallback: Generate clean, distinctive procedural icon and save to
      // cache
      wxImage procImg = GenerateProceduralIcon(procName, cmdLine);
      procImg.SaveFile(cachedFilePath, wxBITMAP_TYPE_PNG);
      wxBitmap bmp(procImg);
      int idx = m_imageList->Add(bmp);
      m_iconIndexMap[key] = idx;
      return idx;
    }

  private:
    ProcessIconManager() : m_imageList(nullptr) {}

    wxImageList *m_imageList;
    wxString m_cacheDir;
    std::map<wxString, wxString> m_desktopMap;
    std::map<wxString, int> m_iconIndexMap;

    void InitCacheDir() {
      wxString home = wxFileName::GetHomeDir();
      m_cacheDir = home + wxFILE_SEP_PATH + ".cache" + wxFILE_SEP_PATH +
                   "sysinfoviewer" + wxFILE_SEP_PATH + "icons";
      if (!wxDirExists(m_cacheDir)) {
        wxFileName::Mkdir(m_cacheDir, wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
      }
    }

    wxString SanitizeFileName(const wxString &name) {
      wxString clean = name;
      clean.Replace("/", "_");
      clean.Replace("\\", "_");
      clean.Replace(":", "_");
      clean.Replace("[", "");
      clean.Replace("]", "");
      clean.Replace(" ", "_");
      return clean;
    }

    void ScanDesktopFiles() {
      wxArrayString searchDirs;
      searchDirs.Add("/usr/share/applications");
      searchDirs.Add("/usr/local/share/applications");
      searchDirs.Add(wxFileName::GetHomeDir() + "/.local/share/applications");
      searchDirs.Add("/var/lib/flatpak/exports/share/applications");
      searchDirs.Add("/var/lib/snapd/desktop/applications");

      for (const auto &dirPath : searchDirs) {
        if (!wxDirExists(dirPath))
          continue;

        wxDir dir(dirPath);
        wxString filename;
        bool cont = dir.GetFirst(&filename, "*.desktop", wxDIR_FILES);
        while (cont) {
          wxString fullPath = dirPath + wxFILE_SEP_PATH + filename;
          ParseDesktopFile(fullPath);
          cont = dir.GetNext(&filename);
        }
      }
    }

    void ParseDesktopFile(const wxString &filePath) {
      wxTextFile file;
      if (!file.Open(filePath))
        return;

      wxString execVal;
      wxString iconVal;
      bool inMainSection = false;

      for (size_t i = 0; i < file.GetLineCount(); ++i) {
        wxString line = file.GetLine(i).Trim(true).Trim(false);
        if (line.StartsWith("[Desktop Entry]")) {
          inMainSection = true;
        } else if (line.StartsWith("[") && inMainSection) {
          break;
        }

        if (inMainSection) {
          if (line.StartsWith("Exec=")) {
            execVal = line.AfterFirst('=').Trim();
          } else if (line.StartsWith("Icon=")) {
            iconVal = line.AfterFirst('=').Trim();
          }
        }
      }
      file.Close();

      if (!execVal.IsEmpty() && !iconVal.IsEmpty()) {
        wxString execBase = execVal;
        if (execBase.StartsWith("env ")) {
          execBase = execBase.Mid(4).Trim(false);
          while (execBase.Contains("=") &&
                 execBase.Find('=') < execBase.Find(' ')) {
            execBase = execBase.AfterFirst(' ').Trim(false);
          }
        }
        size_t spacePos = execBase.Find(' ');
        if (spacePos != wxString::npos) {
          execBase = execBase.Left(spacePos);
        }
        size_t slashPos = execBase.Find('/', true);
        if (slashPos != wxString::npos) {
          execBase = execBase.Mid(slashPos + 1);
        }
        execBase = execBase.Lower().Trim();

        if (!execBase.IsEmpty()) {
          m_desktopMap[execBase] = iconVal;
        }

        wxFileName fn(filePath);
        wxString desktopName = fn.GetName().Lower();
        m_desktopMap[desktopName] = iconVal;
      }
    }

    wxString FindSystemIcon(const wxString &iconName) {
      if (iconName.IsEmpty())
        return "";

      if (iconName.StartsWith("/") && wxFileExists(iconName)) {
        return iconName;
      }

      static const wxArrayString baseDirs = {
          "/usr/share/pixmaps",
          "/usr/share/icons/hicolor/48x48/apps",
          "/usr/share/icons/hicolor/32x32/apps",
          "/usr/share/icons/hicolor/24x24/apps",
          "/usr/share/icons/hicolor/16x16/apps",
          "/usr/share/icons/hicolor/64x64/apps",
          "/usr/share/icons/hicolor/128x128/apps",
          "/usr/share/icons/hicolor/256x256/apps",
          "/usr/share/icons/Adwaita/48x48/apps",
          "/usr/share/icons/Adwaita/32x32/apps",
          "/usr/share/icons/breeze/apps/48",
          "/usr/share/icons/breeze/apps/32",
          "/usr/share/icons/Papirus/48x48/apps",
          "/usr/share/icons/Papirus/32x32/apps"};

      static const wxArrayString exts = {".png", ".xpm", ".ico"};

      for (const auto &dir : baseDirs) {
        if (!wxDirExists(dir))
          continue;

        for (const auto &ext : exts) {
          wxString candidate = dir + wxFILE_SEP_PATH + iconName + ext;
          if (wxFileExists(candidate)) {
            return candidate;
          }
        }
        wxString direct = dir + wxFILE_SEP_PATH + iconName;
        if (wxFileExists(direct)) {
          return direct;
        }
      }

      return "";
    }

    wxImage GenerateProceduralIcon(const wxString &procName,
                                   const wxString &cmdLine) {
      int w = 24, h = 24;
      wxBitmap bmp(w, h, 32);
      wxMemoryDC memDC(bmp);

      wxColour bgColor;
      wxColour fgColor(*wxWHITE);
      wxString label;

      bool isKernel = procName.StartsWith("[") || cmdLine.StartsWith("[");
      bool isShell =
          (procName == "bash" || procName == "zsh" || procName == "sh" ||
           procName == "fish" || procName == "python" ||
           procName == "python3" || procName == "node" || procName == "perl" ||
           procName == "ruby");
      bool isDaemon =
          (procName.EndsWith("d") || procName.Contains("systemd") ||
           procName.Contains("dbus") || procName.Contains("wireplumber") ||
           procName.Contains("pipewire"));

      if (isKernel) {
        bgColor = wxColour(55, 65, 81);
        label = "K";
      } else if (isShell) {
        bgColor = wxColour(30, 41, 59);
        fgColor = wxColour(16, 185, 129);
        label = ">_";
      } else if (isDaemon) {
        bgColor = wxColour(79, 70, 229);
        label = procName.Left(1).Upper();
      } else {
        static const std::vector<wxColour> palette = {
            wxColour(59, 130, 246), // Blue
            wxColour(16, 185, 129), // Emerald
            wxColour(139, 92, 246), // Violet
            wxColour(236, 72, 153), // Pink
            wxColour(245, 158, 11), // Amber
            wxColour(6, 182, 212),  // Cyan
            wxColour(99, 102, 241), // Indigo
            wxColour(20, 184, 166), // Teal
            wxColour(249, 115, 22), // Orange
            wxColour(225, 29, 72),  // Rose
            wxColour(132, 204, 22), // Lime
            wxColour(14, 165, 233)  // Sky
        };
        size_t hashVal = std::hash<std::string>{}(procName.ToStdString());
        bgColor = palette[hashVal % palette.size()];

        wxString clean = procName;
        clean.Trim(false);
        if (!clean.IsEmpty()) {
          label = clean.Left(1).Upper();
        } else {
          label = "P";
        }
      }

      wxGraphicsContext *gc = wxGraphicsContext::Create(memDC);
      if (gc) {
        gc->SetAntialiasMode(wxANTIALIAS_DEFAULT);
        gc->SetBrush(wxBrush(bgColor));
        gc->SetPen(wxPen(bgColor.ChangeLightness(115), 1));
        gc->DrawRoundedRectangle(1.0, 1.0, 22.0, 22.0, 4.0);

        int fontSize = (label.Length() > 1) ? 7 : 9;
        wxFont font(wxFontInfo(fontSize).Bold().Family(wxFONTFAMILY_SWISS));
        gc->SetFont(font, fgColor);
        double tw = 0, th = 0, td = 0, te = 0;
        gc->GetTextExtent(label, &tw, &th, &td, &te);
        double tx = (24.0 - tw) / 2.0;
        double ty = (24.0 - th) / 2.0;
        gc->DrawText(label, tx, ty);
        delete gc;
      }

      return bmp.ConvertToImage();
    }
  };

  class ResourcesInfoPage : public wxPanel {
  public:
    enum {
      ID_PROC_TIMER = wxID_HIGHEST + 200,
      ID_PROC_SEARCH,
      ID_PROC_AUTO_REFRESH,
      ID_PROC_REFRESH,
      ID_PROC_END_PROCESS,
      ID_PROC_SIGTERM,
      ID_PROC_SIGKILL,
      ID_PROC_COPY_PID,
      ID_PROC_COPY_NAME,
      ID_PROC_COPY_CMD,
      ID_PROC_DETAILS
    };

    ResourcesInfoPage(wxPanel *parent)
        : wxPanel(parent, wxID_ANY), m_refreshTimer(this, ID_PROC_TIMER),
          m_sortColumn(3), m_sortAscending(false), m_selectedPid(-1) {
      wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);

      // --- Top Toolbar ---
      wxBoxSizer *toolbarSizer = new wxBoxSizer(wxHORIZONTAL);

      m_searchCtrl =
          new wxSearchCtrl(this, ID_PROC_SEARCH, "", wxDefaultPosition,
                           wxSize(280, -1), wxTE_PROCESS_ENTER);
      m_searchCtrl->SetDescriptiveText(
          "Filter processes by name, PID, user...");
      m_searchCtrl->ShowSearchButton(true);
      m_searchCtrl->ShowCancelButton(true);
      toolbarSizer->Add(m_searchCtrl, 1, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);

      m_autoRefreshCheckBox =
          new wxCheckBox(this, ID_PROC_AUTO_REFRESH, "Auto-refresh (3s)");
      m_autoRefreshCheckBox->SetValue(true);
      toolbarSizer->Add(m_autoRefreshCheckBox, 0,
                        wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);

      wxButton *refreshButton = new wxButton(this, ID_PROC_REFRESH, "Refresh");
      toolbarSizer->Add(refreshButton, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);

      m_endProcessButton =
          new wxButton(this, ID_PROC_END_PROCESS, "End Process");
      toolbarSizer->Add(m_endProcessButton, 0, wxALIGN_CENTER_VERTICAL, 0);

      mainSizer->Add(toolbarSizer, 0, wxEXPAND | wxALL, 6);

      // --- Main List Control ---
      m_listCtrl =
          new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                         wxLC_REPORT | wxLC_SINGLE_SEL | wxBORDER_THEME);

      m_imageList = new wxImageList(24, 24, true);
      m_listCtrl->AssignImageList(m_imageList, wxIMAGE_LIST_SMALL);
      ProcessIconManager::Get().Initialize(m_imageList);

      m_listCtrl->InsertColumn(0, "Process Name", wxLIST_FORMAT_LEFT, 220);
      m_listCtrl->InsertColumn(1, "PID", wxLIST_FORMAT_RIGHT, 80);
      m_listCtrl->InsertColumn(2, "User", wxLIST_FORMAT_LEFT, 95);
      m_listCtrl->InsertColumn(3, "CPU %", wxLIST_FORMAT_RIGHT, 80);
      m_listCtrl->InsertColumn(4, "RAM (RSS)", wxLIST_FORMAT_RIGHT, 105);
      m_listCtrl->InsertColumn(5, "RAM %", wxLIST_FORMAT_RIGHT, 80);
      m_listCtrl->InsertColumn(6, "Time", wxLIST_FORMAT_RIGHT, 90);
      m_listCtrl->InsertColumn(7, "Command Line", wxLIST_FORMAT_LEFT, 360);

      mainSizer->Add(m_listCtrl, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 6);

      // --- Status / Summary Bar ---
      wxBoxSizer *summarySizer = new wxBoxSizer(wxHORIZONTAL);
      m_summaryText = new wxStaticText(this, wxID_ANY, "Loading processes...");
      summarySizer->Add(m_summaryText, 1,
                        wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM,
                        6);
      mainSizer->Add(summarySizer, 0, wxEXPAND);

      SetSizer(mainSizer);

      // Bind events
      m_searchCtrl->Bind(wxEVT_TEXT, &ResourcesInfoPage::OnSearchTextChange,
                         this);
      m_searchCtrl->Bind(wxEVT_SEARCHCTRL_CANCEL_BTN,
                         &ResourcesInfoPage::OnSearchCancel, this);
      m_autoRefreshCheckBox->Bind(
          wxEVT_CHECKBOX, &ResourcesInfoPage::OnAutoRefreshToggle, this);
      refreshButton->Bind(wxEVT_BUTTON, &ResourcesInfoPage::OnRefreshBtn, this);
      m_endProcessButton->Bind(wxEVT_BUTTON,
                               &ResourcesInfoPage::OnEndProcessBtn, this);

      m_listCtrl->Bind(wxEVT_LIST_COL_CLICK, &ResourcesInfoPage::OnColClick,
                       this);
      m_listCtrl->Bind(wxEVT_LIST_ITEM_RIGHT_CLICK,
                       &ResourcesInfoPage::OnItemRightClick, this);
      m_listCtrl->Bind(wxEVT_LIST_ITEM_ACTIVATED,
                       &ResourcesInfoPage::OnItemActivated, this);
      m_listCtrl->Bind(wxEVT_CONTEXT_MENU, &ResourcesInfoPage::OnContextMenu,
                       this);

      Bind(wxEVT_TIMER, &ResourcesInfoPage::OnTimer, this, ID_PROC_TIMER);

      // Context menu event binds
      Bind(wxEVT_MENU, &ResourcesInfoPage::OnMenuSigTerm, this,
           ID_PROC_SIGTERM);
      Bind(wxEVT_MENU, &ResourcesInfoPage::OnMenuSigKill, this,
           ID_PROC_SIGKILL);
      Bind(wxEVT_MENU, &ResourcesInfoPage::OnMenuCopyPid, this,
           ID_PROC_COPY_PID);
      Bind(wxEVT_MENU, &ResourcesInfoPage::OnMenuCopyName, this,
           ID_PROC_COPY_NAME);
      Bind(wxEVT_MENU, &ResourcesInfoPage::OnMenuCopyCmd, this,
           ID_PROC_COPY_CMD);
      Bind(wxEVT_MENU, &ResourcesInfoPage::OnMenuDetails, this,
           ID_PROC_DETAILS);

      // Load initial processes
      RefreshProcessList(false);

      // Start auto-refresh timer (3s)
      m_refreshTimer.Start(3000);
    }

    ~ResourcesInfoPage() {
      if (m_refreshTimer.IsRunning()) {
        m_refreshTimer.Stop();
      }
    }

  private:
    wxSearchCtrl *m_searchCtrl;
    wxCheckBox *m_autoRefreshCheckBox;
    wxButton *m_endProcessButton;
    wxListCtrl *m_listCtrl;
    wxImageList *m_imageList;
    wxStaticText *m_summaryText;
    wxTimer m_refreshTimer;

    std::vector<ProcessInfo> m_allProcesses;
    std::vector<ProcessInfo> m_filteredProcesses;

    int m_sortColumn;
    bool m_sortAscending;
    wxString m_filterText;
    long m_selectedPid;

    static wxString FormatBytesFromKiB(long long kib) {
      if (kib < 1024) {
        return wxString::Format("%lld KiB", kib);
      } else if (kib < 1024 * 1024) {
        return wxString::Format("%.1f MB", static_cast<double>(kib) / 1024.0);
      } else {
        return wxString::Format("%.2f GiB",
                                static_cast<double>(kib) / (1024.0 * 1024.0));
      }
    }

    wxString ExtractProcessName(const wxString &comm, const wxString &command) {
      wxString name = comm;
      name.Trim(true).Trim(false);

      if (name.IsEmpty() || name == command) {
        name = command;
        name.Trim(false);
        if (name.StartsWith("[") && name.Contains("]")) {
          size_t endBracket = name.Find("]");
          if (endBracket != wxString::npos) {
            return name.Left(endBracket + 1);
          }
        }
        size_t spacePos = name.Find(' ');
        if (spacePos != wxString::npos) {
          name = name.Left(spacePos);
        }
        size_t lastSlash = name.Find('/', true);
        if (lastSlash != wxString::npos) {
          name = name.Mid(lastSlash + 1);
        }
      }
      return name;
    }

    void RefreshProcessList(bool preserveSelection = true) {
      if (preserveSelection) {
        long selIndex =
            m_listCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if (selIndex != -1 &&
            selIndex < static_cast<long>(m_filteredProcesses.size())) {
          m_selectedPid = m_filteredProcesses[selIndex].pid;
        } else {
          m_selectedPid = -1;
        }
      }

      m_allProcesses.clear();

      wxArrayString output;
      wxExecute("ps -eo pid,user,pcpu,pmem,rss,time,comm,args --sort=-pcpu",
                output, wxEXEC_SYNC | wxEXEC_NODISABLE);

      for (size_t i = 1; i < output.GetCount(); ++i) {
        wxString line = output[i];
        wxStringTokenizer tokenizer(line, " ", wxTOKEN_STRTOK);
        wxArrayString fields;
        while (tokenizer.HasMoreTokens()) {
          fields.Add(tokenizer.GetNextToken());
        }

        if (fields.size() >= 7) {
          ProcessInfo info;
          info.pid = wxAtol(fields[0]);
          info.user = fields[1];

          double cpu = 0.0;
          fields[2].ToDouble(&cpu);
          info.cpuVal = cpu;
          info.cpuFormatted = wxString::Format("%.1f%%", cpu);

          double ramPerc = 0.0;
          fields[3].ToDouble(&ramPerc);
          info.ramPercentVal = ramPerc;
          info.ramPercentFormatted = wxString::Format("%.1f%%", ramPerc);

          long long rss = 0;
          fields[4].ToLongLong(&rss);
          info.rssKiB = rss;
          info.ramFormatted = FormatBytesFromKiB(rss);

          info.time = fields[5];
          wxString comm = fields[6];

          if (fields.size() > 7) {
            wxArrayString cmdParts;
            for (size_t k = 7; k < fields.size(); ++k) {
              cmdParts.Add(fields[k]);
            }
            info.command = wxJoin(cmdParts, ' ');
          } else {
            info.command = comm;
          }

          info.name = ExtractProcessName(comm, info.command);
          info.iconIndex =
              ProcessIconManager::Get().GetIconIndex(info.name, info.command);

          m_allProcesses.push_back(info);
        }
      }

      FilterAndPopulateList();
    }

    void FilterAndPopulateList() {
      m_filteredProcesses.clear();
      wxString query = m_filterText.Lower().Trim();

      long long totalRssKiB = 0;
      for (const auto &proc : m_allProcesses) {
        totalRssKiB += proc.rssKiB;
        if (query.IsEmpty()) {
          m_filteredProcesses.push_back(proc);
        } else {
          wxString pidStr = wxString::Format("%ld", proc.pid);
          if (proc.name.Lower().Contains(query) ||
              proc.user.Lower().Contains(query) || pidStr.Contains(query) ||
              proc.command.Lower().Contains(query)) {
            m_filteredProcesses.push_back(proc);
          }
        }
      }

      SortProcesses();

      m_listCtrl->Freeze();
      m_listCtrl->DeleteAllItems();

      long restoredIndex = -1;
      for (size_t i = 0; i < m_filteredProcesses.size(); ++i) {
        const auto &proc = m_filteredProcesses[i];
        long itemIdx = m_listCtrl->InsertItem(static_cast<long>(i), proc.name,
                                              proc.iconIndex);
        m_listCtrl->SetItem(itemIdx, 1, wxString::Format("%ld", proc.pid));
        m_listCtrl->SetItem(itemIdx, 2, proc.user);
        m_listCtrl->SetItem(itemIdx, 3, proc.cpuFormatted);
        m_listCtrl->SetItem(itemIdx, 4, proc.ramFormatted);
        m_listCtrl->SetItem(itemIdx, 5, proc.ramPercentFormatted);
        m_listCtrl->SetItem(itemIdx, 6, proc.time);
        m_listCtrl->SetItem(itemIdx, 7, proc.command);

        if (proc.pid == m_selectedPid) {
          restoredIndex = itemIdx;
        }
      }

      if (restoredIndex != -1) {
        m_listCtrl->SetItemState(restoredIndex,
                                 wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED,
                                 wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED);
        m_listCtrl->EnsureVisible(restoredIndex);
      }

      m_listCtrl->Thaw();

      // Column sort name string
      static const char *colNames[] = {"Name",      "PID",   "User", "CPU %",
                                       "RAM (RSS)", "RAM %", "Time", "Command"};
      wxString sortName = (m_sortColumn >= 0 && m_sortColumn < 8)
                              ? colNames[m_sortColumn]
                              : "CPU %";
      wxString sortOrder = m_sortAscending ? "▲ Asc" : "▼ Desc";

      m_summaryText->SetLabel(wxString::Format(
          "Processes: %zu total (%zu displayed)  |  Total Process RAM: %s  |  "
          "Sorted by: %s %s",
          m_allProcesses.size(), m_filteredProcesses.size(),
          FormatBytesFromKiB(totalRssKiB), sortName, sortOrder));
    }

    void SortProcesses() {
      std::sort(m_filteredProcesses.begin(), m_filteredProcesses.end(),
                [this](const ProcessInfo &a, const ProcessInfo &b) {
                  if (m_sortAscending) {
                    switch (m_sortColumn) {
                    case 0:
                      return a.name.CmpNoCase(b.name) < 0;
                    case 1:
                      return a.pid < b.pid;
                    case 2:
                      return a.user.CmpNoCase(b.user) < 0;
                    case 3:
                      return a.cpuVal < b.cpuVal;
                    case 4:
                      return a.rssKiB < b.rssKiB;
                    case 5:
                      return a.ramPercentVal < b.ramPercentVal;
                    case 6:
                      return a.time < b.time;
                    case 7:
                      return a.command.CmpNoCase(b.command) < 0;
                    default:
                      return a.pid < b.pid;
                    }
                  } else {
                    switch (m_sortColumn) {
                    case 0:
                      return a.name.CmpNoCase(b.name) > 0;
                    case 1:
                      return a.pid > b.pid;
                    case 2:
                      return a.user.CmpNoCase(b.user) > 0;
                    case 3:
                      return a.cpuVal > b.cpuVal;
                    case 4:
                      return a.rssKiB > b.rssKiB;
                    case 5:
                      return a.ramPercentVal > b.ramPercentVal;
                    case 6:
                      return a.time > b.time;
                    case 7:
                      return a.command.CmpNoCase(b.command) > 0;
                    default:
                      return a.pid > b.pid;
                    }
                  }
                });
    }

    void OnSearchTextChange(wxCommandEvent &event) {
      m_filterText = event.GetString();
      FilterAndPopulateList();
    }

    void OnSearchCancel(wxCommandEvent &) {
      m_searchCtrl->SetValue("");
      m_filterText = "";
      FilterAndPopulateList();
    }

    void OnAutoRefreshToggle(wxCommandEvent &) {
      if (m_autoRefreshCheckBox->IsChecked()) {
        if (!m_refreshTimer.IsRunning())
          m_refreshTimer.Start(3000);
      } else {
        if (m_refreshTimer.IsRunning())
          m_refreshTimer.Stop();
      }
    }

    void OnRefreshBtn(wxCommandEvent &) { RefreshProcessList(true); }

    void OnTimer(wxTimerEvent &) {
      if (m_autoRefreshCheckBox->IsChecked()) {
        RefreshProcessList(true);
      }
    }

    void OnColClick(wxListEvent &event) {
      int col = event.GetColumn();
      if (col == m_sortColumn) {
        m_sortAscending = !m_sortAscending;
      } else {
        m_sortColumn = col;
        m_sortAscending = (col == 0 || col == 2 || col == 7);
      }
      FilterAndPopulateList();
    }

    long GetSelectedPid() {
      long item =
          m_listCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
      if (item != -1 && item < static_cast<long>(m_filteredProcesses.size())) {
        return m_filteredProcesses[item].pid;
      }
      return -1;
    }

    std::optional<ProcessInfo> GetSelectedProcessInfo() {
      long item =
          m_listCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
      if (item != -1 && item < static_cast<long>(m_filteredProcesses.size())) {
        return m_filteredProcesses[item];
      }
      return std::nullopt;
    }

    void OnEndProcessBtn(wxCommandEvent &) {
      auto infoOpt = GetSelectedProcessInfo();
      if (!infoOpt.has_value()) {
        wxMessageBox("Please select a process from the list first.",
                     "No Process Selected", wxOK | wxICON_INFORMATION, this);
        return;
      }

      const auto &info = infoOpt.value();
      int ans = wxMessageBox(
          wxString::Format("Terminate process '%s' (PID: %ld)?", info.name,
                           info.pid),
          "Confirm Terminate", wxYES_NO | wxNO_DEFAULT | wxICON_WARNING, this);

      if (ans == wxYES) {
        wxKill(info.pid, wxSIGTERM);
        RefreshProcessList(false);
      }
    }

    void OnItemActivated(wxListEvent &event) {
      long idx = event.GetIndex();
      if (idx >= 0 && idx < static_cast<long>(m_filteredProcesses.size())) {
        ShowProcessDetails(m_filteredProcesses[idx]);
      }
    }

    void OnItemRightClick(wxListEvent &event) {
      long idx = event.GetIndex();
      if (idx >= 0) {
        m_listCtrl->SetItemState(idx,
                                 wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED,
                                 wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED);
        ShowContextMenu();
      }
    }

    void OnContextMenu(wxContextMenuEvent &) {
      if (GetSelectedPid() != -1) {
        ShowContextMenu();
      }
    }

    void ShowContextMenu() {
      auto infoOpt = GetSelectedProcessInfo();
      if (!infoOpt.has_value())
        return;

      wxMenu menu;
      menu.Append(ID_PROC_SIGTERM,
                  wxString::Format("&Terminate '%s' (SIGTERM)", infoOpt->name));
      menu.Append(
          ID_PROC_SIGKILL,
          wxString::Format("&Force Kill '%s' (SIGKILL)", infoOpt->name));
      menu.AppendSeparator();
      menu.Append(ID_PROC_COPY_PID, "Copy &PID");
      menu.Append(ID_PROC_COPY_NAME, "Copy Process &Name");
      menu.Append(ID_PROC_COPY_CMD, "Copy Full &Command");
      menu.AppendSeparator();
      menu.Append(ID_PROC_DETAILS, "&Process Details...\tCtrl-D");

      PopupMenu(&menu);
    }

    void OnMenuSigTerm(wxCommandEvent &) {
      auto info = GetSelectedProcessInfo();
      if (info.has_value()) {
        wxKill(info->pid, wxSIGTERM);
        RefreshProcessList(false);
      }
    }

    void OnMenuSigKill(wxCommandEvent &) {
      auto info = GetSelectedProcessInfo();
      if (info.has_value()) {
        int ans = wxMessageBox(
            wxString::Format("Force kill (SIGKILL) process '%s' (PID: %ld)?",
                             info->name, info->pid),
            "Force Kill Confirmation",
            wxYES_NO | wxNO_DEFAULT | wxICON_EXCLAMATION, this);
        if (ans == wxYES) {
          wxKill(info->pid, wxSIGKILL);
          RefreshProcessList(false);
        }
      }
    }

    void OnMenuCopyPid(wxCommandEvent &) {
      auto info = GetSelectedProcessInfo();
      if (info.has_value() && wxTheClipboard->Open()) {
        wxTheClipboard->SetData(
            new wxTextDataObject(wxString::Format("%ld", info->pid)));
        wxTheClipboard->Close();
      }
    }

    void OnMenuCopyName(wxCommandEvent &) {
      auto info = GetSelectedProcessInfo();
      if (info.has_value() && wxTheClipboard->Open()) {
        wxTheClipboard->SetData(new wxTextDataObject(info->name));
        wxTheClipboard->Close();
      }
    }

    void OnMenuCopyCmd(wxCommandEvent &) {
      auto info = GetSelectedProcessInfo();
      if (info.has_value() && wxTheClipboard->Open()) {
        wxTheClipboard->SetData(new wxTextDataObject(info->command));
        wxTheClipboard->Close();
      }
    }

    void OnMenuDetails(wxCommandEvent &) {
      auto info = GetSelectedProcessInfo();
      if (info.has_value()) {
        ShowProcessDetails(info.value());
      }
    }

    void ShowProcessDetails(const ProcessInfo &info) {
      long pid = info.pid;
      wxDialog dlg(
          this, wxID_ANY,
          wxString::Format("Process Details - %s (PID: %ld)", info.name, pid),
          wxDefaultPosition, wxSize(580, 500),
          wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
      wxBoxSizer *dlgSizer = new wxBoxSizer(wxVERTICAL);

      wxTextCtrl *detailsText = new wxTextCtrl(
          &dlg, wxID_ANY, "", wxDefaultPosition, wxDefaultSize,
          wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH2 | wxHSCROLL);

      wxFont monoFont(wxFontInfo(10).Family(wxFONTFAMILY_TELETYPE));
      detailsText->SetFont(monoFont);

      wxString content;
      content += "==================================================\n";
      content += wxString::Format("  PROCESS: %s (PID: %ld)\n", info.name, pid);
      content += "==================================================\n\n";

      content += wxString::Format("User:          %s\n", info.user);
      content += wxString::Format("CPU Usage:     %s\n", info.cpuFormatted);
      content += wxString::Format("RAM (RSS):     %s (%s)\n", info.ramFormatted,
                                  info.ramPercentFormatted);
      content += wxString::Format("CPU Time:      %s\n\n", info.time);

      char exeBuf[PATH_MAX];
      ssize_t exeLen = readlink(wxString::Format("/proc/%ld/exe", pid).c_str(),
                                exeBuf, sizeof(exeBuf) - 1);
      if (exeLen > 0) {
        exeBuf[exeLen] = '\0';
        content += wxString::Format("Binary Path:   %s\n", exeBuf);
      }

      char cwdBuf[PATH_MAX];
      ssize_t cwdLen = readlink(wxString::Format("/proc/%ld/cwd", pid).c_str(),
                                cwdBuf, sizeof(cwdBuf) - 1);
      if (cwdLen > 0) {
        cwdBuf[cwdLen] = '\0';
        content += wxString::Format("Working Dir:   %s\n\n", cwdBuf);
      }

      content += "Status Metrics (/proc/[pid]/status):\n";
      content += "------------------------------------\n";
      wxTextFile statusFile;
      if (statusFile.Open(wxString::Format("/proc/%ld/status", pid))) {
        for (size_t i = 0; i < statusFile.GetLineCount(); ++i) {
          wxString line = statusFile.GetLine(i);
          if (line.StartsWith("State:") || line.StartsWith("Tgid:") ||
              line.StartsWith("PPid:") || line.StartsWith("Uid:") ||
              line.StartsWith("Gid:") || line.StartsWith("Threads:") ||
              line.StartsWith("VmPeak:") || line.StartsWith("VmSize:") ||
              line.StartsWith("VmRSS:") || line.StartsWith("VmData:") ||
              line.StartsWith("VmStk:") || line.StartsWith("VmExe:") ||
              line.StartsWith("VmLib:")) {
            content += "  " + line + "\n";
          }
        }
        statusFile.Close();
      }

      content += wxString::Format(
          "\nFull Command Line:\n------------------\n%s\n", info.command);

      detailsText->SetValue(content);
      dlgSizer->Add(detailsText, 1, wxEXPAND | wxALL, 10);

      wxStdDialogButtonSizer *btnSizer = dlg.CreateStdDialogButtonSizer(wxOK);
      dlgSizer->Add(btnSizer, 0, wxALIGN_RIGHT | wxBOTTOM | wxRIGHT, 10);

      dlg.SetSizer(dlgSizer);
      dlg.CenterOnParent();
      dlg.ShowModal();
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

  // Ensure sensible minimum size so all panes remain accessible
  networkPane->SetMinSize(wxSize(220, 260));
  displayInfoPane->SetMinSize(wxSize(220, 260));
  storageDevicesPane->SetMinSize(wxSize(220, 260));
  audioDevicesPane->SetMinSize(wxSize(220, 260));
  motherboardInfoPane->SetMinSize(wxSize(220, 260));
  cpuInfoPane->SetMinSize(wxSize(220, 260));

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
  // ----------------- DISPLAY INFORMATION -------------
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

    struct EdidInfo {
      wxString manufacturer;
      wxString model;
      std::optional<wxString> screenSizeInches;
      std::optional<wxString> refreshRate;
    };

    int m_baseFontSize = 10;
    std::vector<wxStaticText *> m_infoLabels;

    // ────────────────────────────────────────────────────────────────
    void CalculateBaseFontSize() {
      wxDisplay display(0u);
      wxSize ppi = display.GetPPI();
      m_baseFontSize = wxMax(10, wxMin(ppi.GetWidth() / 10, 16));
      if (ppi.GetWidth() > 120)
        m_baseFontSize += 2;
    }

    void OnSize([[maybe_unused]] wxSizeEvent &event) {
      UpdateFontSizes();
      event.Skip();
    }

    void UpdateFontSizes() {
      int windowWidth = GetSize().GetWidth();
      int dynamicSize = wxMax(m_baseFontSize - 2,
                              wxMin(m_baseFontSize + 2, windowWidth / 50));
      wxFont font = GetFont();
      font.SetPointSize(dynamicSize);
      for (auto *label : m_infoLabels)
        label->SetFont(font);
      Layout();
    }

    // ────────────────────────────────────────────────────────────────
    void CollectSystemInfo() {
      m_systemInfo.desktopEnv = GetDesktopEnvironment();

      try {
        m_systemInfo.displayCount = wxDisplay::GetCount();
      } catch (...) {
        m_systemInfo.displayCount = 0;
      }

      // DRM sysfs — resolution, works on all DEs and display servers
      ParseDrmSysfs();

      // Fallback resolution from wxDisplay
      if (!m_systemInfo.resolution && m_systemInfo.displayCount > 0) {
        wxDisplay display(0u);
        wxRect geom = display.GetGeometry();
        m_systemInfo.resolution =
            wxString::Format("%dx%d", geom.width, geom.height);
      }

      // Color depth from wxDisplay
      if (m_systemInfo.displayCount > 0) {
        try {
          wxDisplay display(0u);
          wxVideoMode mode = display.GetCurrentMode();
          if (mode.GetDepth() > 0)
            m_systemInfo.colorDepth = mode.GetDepth();
        } catch (...) {
        }
      }

      // Brightness from sysfs backlight
      m_systemInfo.brightness = GetBrightness();

      // EDID — physical size, refresh rate, manufacturer, model
      // Pure DRM/sysfs — no DE dependency, works on X11, Wayland,
      // GNOME, KDE, Cinnamon, Pantheon, XFCE, anything
      auto edidInfo = GetEdidInfo();
      if (edidInfo) {
        if (!edidInfo->manufacturer.IsEmpty())
          m_systemInfo.manufacturer = edidInfo->manufacturer;
        if (!edidInfo->model.IsEmpty())
          m_systemInfo.model = edidInfo->model;
        if (edidInfo->screenSizeInches.has_value())
          m_systemInfo.screenSizeInches = edidInfo->screenSizeInches;
        if (edidInfo->refreshRate.has_value())
          m_systemInfo.refreshRate = edidInfo->refreshRate;
      }
    }

    // ────────────────────────────────────────────────────────────────
    // Read resolution from DRM sysfs connector modes file.
    // Works on X11, Wayland, any DE — no xrandr needed.
    void ParseDrmSysfs() {
      namespace fs = std::filesystem;
      const fs::path drm("/sys/class/drm");
      if (!fs::exists(drm))
        return;

      // Priority: eDP (internal panel) > HDMI > DP > anything else
      auto priority = [](const std::string &name) -> int {
        if (name.find("eDP") != std::string::npos)
          return 0;
        if (name.find("HDMI") != std::string::npos)
          return 1;
        if (name.find("DP") != std::string::npos)
          return 2;
        return 3;
      };

      struct Connector {
        fs::path path;
        int pri;
      };
      std::vector<Connector> connected;

      std::error_code ec;
      for (const auto &entry : fs::directory_iterator(drm, ec)) {
        std::string name = entry.path().filename().string();
        // Connector dirs always contain a hyphen e.g. card1-eDP-1
        if (name.find('-') == std::string::npos)
          continue;

        fs::path statusFile = entry.path() / "status";
        if (!fs::exists(statusFile, ec))
          continue;

        std::ifstream sf(statusFile);
        std::string state;
        if (std::getline(sf, state) && state == "connected")
          connected.push_back({entry.path(), priority(name)});
      }

      if (connected.empty())
        return;

      std::sort(
          connected.begin(), connected.end(),
          [](const Connector &a, const Connector &b) { return a.pri < b.pri; });

      // Use highest-priority connected connector
      fs::path modesFile = connected[0].path / "modes";
      if (fs::exists(modesFile, ec)) {
        std::ifstream mf(modesFile);
        std::string firstMode;
        if (std::getline(mf, firstMode) && !firstMode.empty())
          m_systemInfo.resolution = wxString::FromUTF8(firstMode);
      }
    }

    // ────────────────────────────────────────────────────────────────
    std::optional<wxString> GetDesktopEnvironment() {
      // Environment variables — most reliable across all DEs
      for (const char *var :
           {"XDG_CURRENT_DESKTOP", "DESKTOP_SESSION", "GDMSESSION"}) {
        const char *val = std::getenv(var);
        if (val && val[0] != '\0')
          return wxString::FromUTF8(val);
      }
      // Fallback: read /etc/os-release directly (no shell pipe)
      std::ifstream f("/etc/os-release");
      std::string line;
      while (std::getline(f, line)) {
        if (line.rfind("PRETTY_NAME=", 0) == 0) {
          std::string val = line.substr(12);
          if (val.size() >= 2 && val.front() == '"')
            val = val.substr(1, val.size() - 2);
          return wxString::FromUTF8(val);
        }
      }
      return std::nullopt;
    }

    // ────────────────────────────────────────────────────────────────
    std::optional<int> GetBrightness() {
      namespace fs = std::filesystem;
      const fs::path backlight("/sys/class/backlight");
      if (!fs::exists(backlight))
        return std::nullopt;

      std::error_code ec;
      for (const auto &entry : fs::directory_iterator(backlight, ec)) {
        if (!entry.is_directory(ec))
          continue;
        fs::path cur = entry.path() / "brightness";
        fs::path max = entry.path() / "max_brightness";
        if (!fs::exists(cur, ec) || !fs::exists(max, ec))
          continue;

        std::ifstream fc(cur), fm(max);
        long cv = 0, mv = 0;
        if ((fc >> cv) && (fm >> mv) && mv > 0)
          return static_cast<int>((static_cast<double>(cv) / mv) * 100.0);
      }
      return std::nullopt;
    }

    // ────────────────────────────────────────────────────────────────
    bool GetEdidFromDrm(std::vector<unsigned char> &edid_data) {
      for (int i = 0; i < 8; ++i) {
        std::string path = "/dev/dri/card" + std::to_string(i);
        int fd = open(path.c_str(), O_RDWR | O_CLOEXEC);
        if (fd < 0)
          continue;

        drmModeRes *res = drmModeGetResources(fd);
        if (!res) {
          close(fd);
          continue;
        }

        bool found = false;
        for (int j = 0; j < res->count_connectors && !found; ++j) {
          drmModeConnector *conn = drmModeGetConnector(fd, res->connectors[j]);
          if (!conn)
            continue;

          if (conn->connection == DRM_MODE_CONNECTED) {
            for (int k = 0; k < conn->count_props && !found; ++k) {
              drmModePropertyRes *prop = drmModeGetProperty(fd, conn->props[k]);
              if (!prop)
                continue;

              if (strcmp(prop->name, "EDID") == 0) {
                drmModePropertyBlobRes *blob =
                    drmModeGetPropertyBlob(fd, conn->prop_values[k]);
                if (blob && blob->length >= 128) {
                  edid_data.assign(static_cast<unsigned char *>(blob->data),
                                   static_cast<unsigned char *>(blob->data) +
                                       blob->length);
                  found = true;
                }
                if (blob)
                  drmModeFreePropertyBlob(blob);
              }
              drmModeFreeProperty(prop);
            }
          }
          drmModeFreeConnector(conn);
        }
        drmModeFreeResources(res);
        close(fd);
        if (found)
          return true;
      }
      return false;
    }

    // ────────────────────────────────────────────────────────────────
    std::optional<EdidInfo> GetEdidInfo() {
      std::vector<unsigned char> edid;
      if (!GetEdidFromDrm(edid) || edid.size() < 128)
        return std::nullopt;

      // Validate EDID header magic
      const unsigned char magic[8] = {0x00, 0xFF, 0xFF, 0xFF,
                                      0xFF, 0xFF, 0xFF, 0x00};
      if (std::memcmp(edid.data(), magic, 8) != 0)
        return std::nullopt;

      EdidInfo info;

      // Manufacturer ID — bytes 8-9, ISA PnP packed ASCII
      char mfr[4] = {
          static_cast<char>(((edid[8] >> 2) & 0x1F) + 'A' - 1),
          static_cast<char>(
              (((edid[8] & 0x03) << 3) | ((edid[9] >> 5) & 0x07)) + 'A' - 1),
          static_cast<char>((edid[9] & 0x1F) + 'A' - 1), '\0'};
      info.manufacturer = wxString::FromUTF8(mfr);

      // Physical size — bytes 21-22 in cm → diagonal inches
      int w_cm = edid[21];
      int h_cm = edid[22];
      if (w_cm > 0 && h_cm > 0) {
        double w_mm = w_cm * 10.0;
        double h_mm = h_cm * 10.0;
        double diag = std::sqrt(w_mm * w_mm + h_mm * h_mm) / 25.4;
        info.screenSizeInches = wxString::Format("%.1f\"", diag);
      }

      // Detailed timing descriptors — bytes 54-125, 4 × 18 bytes
      for (int i = 54; i + 17 < 128; i += 18) {
        uint16_t pixel_clock_raw = static_cast<uint16_t>(edid[i]) |
                                   (static_cast<uint16_t>(edid[i + 1]) << 8);

        // Non-zero pixel clock → timing descriptor → compute refresh rate
        if (pixel_clock_raw > 0 && !info.refreshRate.has_value()) {
          uint32_t pixel_clock = pixel_clock_raw * 10000UL;

          uint32_t h_active =
              edid[i + 2] | ((static_cast<uint32_t>(edid[i + 4] >> 4)) << 8);
          uint32_t h_blank =
              edid[i + 3] | ((static_cast<uint32_t>(edid[i + 4] & 0x0F)) << 8);
          uint32_t v_active =
              edid[i + 5] | ((static_cast<uint32_t>(edid[i + 7] >> 4)) << 8);
          uint32_t v_blank =
              edid[i + 6] | ((static_cast<uint32_t>(edid[i + 7] & 0x0F)) << 8);

          uint32_t h_total = h_active + h_blank;
          uint32_t v_total = v_active + v_blank;

          if (h_total > 0 && v_total > 0) {
            double refresh =
                static_cast<double>(pixel_clock) /
                (static_cast<double>(h_total) * static_cast<double>(v_total));
            info.refreshRate = wxString::Format("%.2f", refresh);
          }
        }

        // Monitor name (0xFC) or text (0xFE) descriptor
        if (edid[i] == 0x00 && edid[i + 1] == 0x00 && edid[i + 2] == 0x00 &&
            info.model.IsEmpty() &&
            (edid[i + 3] == 0xFC || edid[i + 3] == 0xFE)) {
          wxString name;
          for (int j = i + 5; j < i + 18; ++j) {
            if (edid[j] == 0x0A)
              break;
            if (edid[j] >= 0x20 && edid[j] <= 0x7E)
              name += static_cast<char>(edid[j]);
          }
          name = name.Trim();
          if (!name.IsEmpty())
            info.model = name;
        }
      }

      return info;
    }

    // ────────────────────────────────────────────────────────────────
    void CreateInfoDisplay() {
      wxSizer *sizer = GetSizer();
      wxFont boldFont = GetFont();
      boldFont.MakeBold();
      boldFont.SetPointSize(m_baseFontSize);

      // Skips the row entirely if value is empty
      auto AddInfoLine = [&](const wxString &label, const wxString &value) {
        if (value.IsEmpty())
          return;
        wxBoxSizer *row = new wxBoxSizer(wxHORIZONTAL);
        wxStaticText *key = new wxStaticText(this, wxID_ANY, label + ":");
        key->SetFont(boldFont);
        wxStaticText *val = new wxStaticText(this, wxID_ANY, value);
        row->Add(key, 0, wxALL, 4);
        row->Add(val, 1, wxALL, 4);
        sizer->Add(row, 0, wxEXPAND | wxLEFT | wxRIGHT, 5);
        m_infoLabels.push_back(key);
        m_infoLabels.push_back(val);
      };

      if (m_systemInfo.desktopEnv)
        AddInfoLine("Desktop", *m_systemInfo.desktopEnv);

      AddInfoLine("Displays",
                  wxString::Format("%u", m_systemInfo.displayCount));

      if (m_systemInfo.resolution)
        AddInfoLine("Resolution", *m_systemInfo.resolution);

      if (m_systemInfo.screenSizeInches)
        AddInfoLine("Screen Size", *m_systemInfo.screenSizeInches);

      if (m_systemInfo.refreshRate)
        AddInfoLine("Refresh Rate", *m_systemInfo.refreshRate + " Hz");

      if (m_systemInfo.colorDepth)
        AddInfoLine("Color Depth",
                    wxString::Format("%d bit", *m_systemInfo.colorDepth));

      if (m_systemInfo.brightness)
        AddInfoLine("Brightness",
                    wxString::Format("%d%%", *m_systemInfo.brightness));

      if (m_systemInfo.manufacturer)
        AddInfoLine("Manufacturer", *m_systemInfo.manufacturer);

      if (m_systemInfo.model && !m_systemInfo.model->IsEmpty())
        AddInfoLine("Model", *m_systemInfo.model);

      UpdateFontSizes();
    }
  };

  // Add DISPLAY INFORMATION to the top center of page 3
  DisplayInfoPanel *displayInfo = new DisplayInfoPanel(displayInfoPane);
  displaySizer->Add(displayInfo, 1, wxEXPAND | wxALL, 10);
  displayInfoPane->SetSizer(displaySizer);
  displayInfoPane->Layout();

  // ------------------ DISPLAY END =====================

  // ---------------------------- STORAGE DEVICES
  // ---------------------------------------------
  wxStaticBoxSizer *storageSizer = new wxStaticBoxSizer(
      wxVERTICAL, storageDevicesPane, "STORAGE INFORMATION");

  class StorageUsageBar : public wxPanel {
  public:
    StorageUsageBar(wxWindow *parent, wxWindowID id = wxID_ANY,
                    double percentage = 0.0,
                    const wxPoint &pos = wxDefaultPosition,
                    const wxSize &size = wxSize(-1, 20))
        : wxPanel(parent, id, pos, size), m_percentage(percentage) {
      SetMinSize(wxSize(-1, 20));
      Bind(wxEVT_PAINT, &StorageUsageBar::OnPaint, this);
    }

    void SetPercentage(double pct) {
      m_percentage = wxMax(0.0, wxMin(100.0, pct));
      Refresh();
    }

  private:
    double m_percentage;

    void OnPaint([[maybe_unused]] wxPaintEvent &event) {
      wxPaintDC dc(this);
      wxSize sz = GetClientSize();
      if (sz.GetWidth() <= 0 || sz.GetHeight() <= 0)
        return;

      wxGraphicsContext *gc = wxGraphicsContext::Create(dc);
      if (!gc)
        return;

      gc->SetAntialiasMode(wxANTIALIAS_DEFAULT);

      // Track / Background
      wxColour trackCol(226, 232, 240); // Slate #E2E8F0
      gc->SetBrush(wxBrush(trackCol));
      gc->SetPen(*wxTRANSPARENT_PEN);
      gc->DrawRoundedRectangle(0, 0, sz.GetWidth(), sz.GetHeight(), 4.0);

      // Filled portion
      double fillWidth = (m_percentage / 100.0) * sz.GetWidth();
      if (fillWidth > 2.0) {
        wxColour fillCol;
        if (m_percentage >= 90.0) {
          fillCol = wxColour(239, 68, 68); // Red #EF4444
        } else if (m_percentage >= 70.0) {
          fillCol = wxColour(245, 158, 11); // Amber #F59E0B
        } else {
          fillCol = wxColour(16, 185, 129); // Emerald #10B981
        }
        gc->SetBrush(wxBrush(fillCol));
        gc->DrawRoundedRectangle(0, 0, fillWidth, sz.GetHeight(), 4.0);
      }

      // Percentage label text
      wxString pctText = wxString::Format("%.1f%%", m_percentage);
      wxFont font(wxFontInfo(9).Bold().Family(wxFONTFAMILY_SWISS));
      gc->SetFont(font, (m_percentage > 55.0) ? *wxWHITE : wxColour(30, 41, 59));
      double tw = 0, th = 0, td = 0, te = 0;
      gc->GetTextExtent(pctText, &tw, &th, &td, &te);
      double tx = (sz.GetWidth() - tw) / 2.0;
      double ty = (sz.GetHeight() - th) / 2.0;
      gc->DrawText(pctText, tx, ty);

      delete gc;
    }
  };

  class StorageDevicesPanel : public wxScrolledWindow {
  public:
    StorageDevicesPanel(wxWindow *parent, wxWindowID id = wxID_ANY)
        : wxScrolledWindow(parent, id, wxDefaultPosition, wxDefaultSize,
                           wxVSCROLL) {
      SetScrollRate(0, 10);
      m_mainSizer = new wxBoxSizer(wxVERTICAL);
      SetSizer(m_mainSizer);

      PopulateStorageDevices();

      // Periodic check for drive insertion / removal (every 4s)
      m_timer.Bind(wxEVT_TIMER, &StorageDevicesPanel::OnTimer, this);
      m_timer.Start(4000);
    }

  private:
    struct PhysicalStorageDevice {
      wxString name;
      wxString devPath;
      wxString model;
      wxString vendor;
      wxString tran;
      wxString deviceType;
      wxString badgeIcon;
      bool isRotational = false;
      bool isRemovable = false;

      double totalGB = 0.0;
      double usedGB = 0.0;
      double freeGB = 0.0;
      double usedPercentage = 0.0;

      wxArrayString mountPoints;
      wxArrayString fsTypes;
    };

    wxBoxSizer *m_mainSizer = nullptr;
    wxTimer m_timer;
    std::vector<PhysicalStorageDevice> m_cachedDevices;

    void OnTimer([[maybe_unused]] wxTimerEvent &event) {
      std::vector<PhysicalStorageDevice> current = GetPhysicalStorageDevices();
      bool changed = (current.size() != m_cachedDevices.size());
      if (!changed) {
        for (size_t i = 0; i < current.size(); ++i) {
          if (current[i].name != m_cachedDevices[i].name ||
              std::abs(current[i].usedPercentage -
                       m_cachedDevices[i].usedPercentage) > 0.5) {
            changed = true;
            break;
          }
        }
      }
      if (changed) {
        PopulateStorageDevices();
      }
    }

    void PopulateStorageDevices() {
      Freeze();
      m_mainSizer->Clear(true);
      m_cachedDevices = GetPhysicalStorageDevices();

      if (m_cachedDevices.empty()) {
        wxStaticText *errorText =
            new wxStaticText(this, wxID_ANY,
                             "No physical storage devices found or unable to "
                             "retrieve storage information.");
        m_mainSizer->Add(errorText, 0, wxALL, 10);
      } else {
        for (size_t i = 0; i < m_cachedDevices.size(); ++i) {
          const auto &info = m_cachedDevices[i];

          // Header: icon + model (safe concatenation, no Format with emoji)
          wxString headerStr = info.badgeIcon + wxString("  ") + info.model;
          wxStaticBoxSizer *deviceBox =
              new wxStaticBoxSizer(wxVERTICAL, this, headerStr);

          // Sub-header info: device node & type (• is multi-byte, avoid Format)
          wxString typeStr = wxString("Node: ") + info.devPath +
                             wxString::FromUTF8("  \xE2\x80\xA2  Type: ") +
                             info.deviceType +
                             wxString::FromUTF8("  \xE2\x80\xA2  ") +
                             wxString(info.isRemovable ? "Removable" : "Internal");
          wxStaticText *typeText = new wxStaticText(this, wxID_ANY, typeStr);
          wxFont subFont = typeText->GetFont();
          subFont.SetPointSize(wxMax(8, subFont.GetPointSize() - 1));
          typeText->SetFont(subFont);
          typeText->SetForegroundColour(wxColour(100, 116, 139));
          deviceBox->Add(typeText, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 4);

          // Storage usage bar
          StorageUsageBar *gauge =
              new StorageUsageBar(this, wxID_ANY, info.usedPercentage);
          deviceBox->Add(gauge, 0, wxEXPAND | wxALL, 4);

          // Storage metrics line
          wxString metricsStr = wxString::Format(
              "Used: %.2f GB (%.1f%%)   |   Free: %.2f GB   |   Total: %.2f GB",
              info.usedGB, info.usedPercentage, info.freeGB, info.totalGB);
          wxStaticText *metricsText =
              new wxStaticText(this, wxID_ANY, metricsStr);
          metricsText->SetFont(metricsText->GetFont().Bold());
          deviceBox->Add(metricsText, 0,
                         wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 4);

          // Mount points summary
          if (!info.mountPoints.IsEmpty()) {
            wxString mountStr = "Mounts: ";
            for (size_t mi = 0; mi < info.mountPoints.GetCount(); ++mi) {
              if (mi > 0) mountStr += ", ";
              mountStr += info.mountPoints[mi];
            }
            if (!info.fsTypes.IsEmpty()) {
              mountStr += " (";
              for (size_t fi = 0; fi < info.fsTypes.GetCount(); ++fi) {
                if (fi > 0) mountStr += ", ";
                mountStr += info.fsTypes[fi];
              }
              mountStr += ")";
            }
            wxStaticText *mountText =
                new wxStaticText(this, wxID_ANY, mountStr);
            mountText->SetFont(subFont);
            deviceBox->Add(mountText, 0,
                           wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 4);
          }

          m_mainSizer->Add(deviceBox, 0, wxEXPAND | wxALL, 5);
        }
      }

      Layout();
      FitInside();
      Thaw();
    }

    std::vector<PhysicalStorageDevice> GetPhysicalStorageDevices() {
      std::vector<PhysicalStorageDevice> devices;
      wxArrayString output;
      long exitCode = wxExecute(
          "lsblk -b -P -o "
          "NAME,PKNAME,TYPE,MODEL,VENDOR,SIZE,ROTA,RM,MOUNTPOINTS,FSUSED,FSAVAIL,FSSIZE,FSTYPE,TRAN",
          output, wxEXEC_SYNC | wxEXEC_NODISABLE);

      if (exitCode == 0 && !output.IsEmpty()) {
        struct BlockItem {
          wxString name;
          wxString pkName;
          wxString type;
          wxString model;
          wxString vendor;
          long long size = 0;
          bool rota = false;
          bool rm = false;
          wxArrayString mountpoints;
          long long fsused = -1;
          long long fsavail = -1;
          wxString fstype;
          wxString tran;
        };

        std::vector<BlockItem> allItems;
        for (const auto &line : output) {
          auto kv = ParseLsblkLine(line);
          BlockItem item;
          item.name = kv["NAME"];
          item.pkName = kv["PKNAME"];
          item.type = kv["TYPE"];
          item.model = kv["MODEL"];
          item.vendor = kv["VENDOR"];
          kv["SIZE"].ToLongLong(&item.size);
          item.rota = (kv["ROTA"] == "1");
          item.rm = (kv["RM"] == "1");
          item.fstype = kv["FSTYPE"];
          item.tran = kv["TRAN"].Lower();

          wxString mountsRaw = kv["MOUNTPOINTS"];
          wxStringTokenizer mTok(mountsRaw, "\n");
          while (mTok.HasMoreTokens()) {
            wxString m = mTok.GetNextToken().Trim(true).Trim(false);
            if (!m.IsEmpty() && m != "[SWAP]") {
              item.mountpoints.Add(m);
            }
          }

          if (kv.find("FSUSED") != kv.end() && !kv["FSUSED"].IsEmpty()) {
            kv["FSUSED"].ToLongLong(&item.fsused);
          }
          if (kv.find("FSAVAIL") != kv.end() && !kv["FSAVAIL"].IsEmpty()) {
            kv["FSAVAIL"].ToLongLong(&item.fsavail);
          }

          allItems.push_back(item);
        }

        // Process only physical disk items (TYPE == "disk")
        for (const auto &item : allItems) {
          if (item.type != "disk")
            continue;

          // Skip virtual and zram/loop/ram devices
          if (item.name.StartsWith("zram") || item.name.StartsWith("loop") ||
              item.name.StartsWith("ram") || item.name.StartsWith("dm-")) {
            continue;
          }

          PhysicalStorageDevice dev;
          dev.name = item.name;
          dev.devPath = "/dev/" + item.name;
          { wxString v = item.vendor; dev.vendor = v.Trim(); }
          dev.tran = item.tran;
          dev.isRotational = item.rota;
          dev.isRemovable = item.rm;

          // Determine Model
          { wxString m = item.model; dev.model = m.Trim(); }
          if (dev.model.IsEmpty()) {
            wxString sysModelPath =
                wxString::Format("/sys/block/%s/device/model", item.name);
            if (wxFileExists(sysModelPath)) {
              wxTextFile tf;
              if (tf.Open(sysModelPath)) {
                dev.model = tf.GetFirstLine().Trim();
                tf.Close();
              }
            }
            if (dev.model.IsEmpty()) {
              wxString sysNamePath =
                  wxString::Format("/sys/block/%s/device/name", item.name);
              if (wxFileExists(sysNamePath)) {
                wxTextFile tf;
                if (tf.Open(sysNamePath)) {
                  dev.model = tf.GetFirstLine().Trim();
                  tf.Close();
                }
              }
            }
          }

          // Determine device type & badge
          if (dev.tran == "nvme" || dev.name.StartsWith("nvme")) {
            dev.deviceType = "NVMe Solid State Drive (SSD)";
            dev.badgeIcon = wxString::FromUTF8("[NVMe SSD]");
          } else if (dev.name.StartsWith("mmcblk")) {
            dev.deviceType = "SD / Memory Card";
            dev.badgeIcon = wxString::FromUTF8("[SD/MMC]");
          } else if (dev.isRemovable || dev.tran == "usb") {
            dev.deviceType = "USB Flash / External Drive";
            dev.badgeIcon = wxString::FromUTF8("[USB]");
          } else if (dev.isRotational) {
            dev.deviceType = "Hard Disk Drive (HDD)";
            dev.badgeIcon = wxString::FromUTF8("[HDD]");
          } else {
            dev.deviceType = "SATA Solid State Drive (SSD)";
            dev.badgeIcon = wxString::FromUTF8("[SATA SSD]");
          }

          if (dev.model.IsEmpty()) {
            dev.model = dev.deviceType;
          }

          dev.totalGB =
              static_cast<double>(item.size) / (1024.0 * 1024.0 * 1024.0);

          // Find all child partitions under this disk
          long long totalUsedBytes = 0;
          std::set<wxString> countedPartitions;
          std::set<wxString> uniqueMounts;
          std::set<wxString> uniqueFs;

          std::function<void(const wxString &)> collectChildren =
              [&](const wxString &parentName) {
                for (const auto &child : allItems) {
                  if (child.pkName == parentName) {
                    if (!child.fstype.IsEmpty() &&
                        child.fstype != "crypto_LUKS") {
                      uniqueFs.insert(child.fstype);
                    }
                    for (const auto &m : child.mountpoints) {
                      uniqueMounts.insert(m);
                    }

                    if (countedPartitions.find(child.name) ==
                        countedPartitions.end()) {
                      countedPartitions.insert(child.name);
                      if (child.fsused > 0) {
                        totalUsedBytes += child.fsused;
                      } else if (!child.mountpoints.IsEmpty()) {
                        struct statvfs st;
                        if (statvfs(child.mountpoints[0].mb_str(), &st) == 0) {
                          long long used =
                              (static_cast<long long>(st.f_blocks) -
                               static_cast<long long>(st.f_bfree)) *
                              st.f_frsize;
                          if (used > 0)
                            totalUsedBytes += used;
                        }
                      }
                    }

                    collectChildren(child.name);
                  }
                }
              };

          collectChildren(item.name);

          dev.usedGB = static_cast<double>(totalUsedBytes) /
                       (1024.0 * 1024.0 * 1024.0);
          dev.freeGB = (dev.totalGB > dev.usedGB)
                           ? (dev.totalGB - dev.usedGB)
                           : 0.0;
          dev.usedPercentage = (dev.totalGB > 0.0)
                                   ? ((dev.usedGB / dev.totalGB) * 100.0)
                                   : 0.0;

          for (const auto &m : uniqueMounts)
            dev.mountPoints.Add(m);
          for (const auto &f : uniqueFs)
            dev.fsTypes.Add(f);

          devices.push_back(dev);
        }
      }

      if (devices.empty()) {
        devices = GetFallbackStorageDevices();
      }

      return devices;
    }

    std::map<wxString, wxString> ParseLsblkLine(const wxString &line) {
      std::map<wxString, wxString> map;
      size_t pos = 0;
      while (pos < line.length()) {
        size_t eqPos = line.find('=', pos);
        if (eqPos == wxString::npos)
          break;
        wxString key = line.substr(pos, eqPos - pos).Trim(true).Trim(false);
        if (eqPos + 1 < line.length() && line[eqPos + 1] == '"') {
          size_t endQuote = line.find('"', eqPos + 2);
          if (endQuote == wxString::npos)
            break;
          wxString val = line.substr(eqPos + 2, endQuote - (eqPos + 2));
          val.Replace("\\x0a", "\n");
          map[key] = val;
          pos = endQuote + 1;
        } else {
          size_t spacePos = line.find(' ', eqPos + 1);
          if (spacePos == wxString::npos)
            spacePos = line.length();
          wxString val = line.substr(eqPos + 1, spacePos - (eqPos + 1));
          map[key] = val;
          pos = spacePos + 1;
        }
        while (pos < line.length() && (line[pos] == ' ' || line[pos] == '\t'))
          pos++;
      }
      return map;
    }

    std::vector<PhysicalStorageDevice> GetFallbackStorageDevices() {
      std::vector<PhysicalStorageDevice> devices;
      struct statvfs stat;
      if (statvfs("/", &stat) == 0) {
        double totalBytes = static_cast<double>(stat.f_frsize) * stat.f_blocks;
        double freeBytes = static_cast<double>(stat.f_frsize) * stat.f_bfree;
        double usedBytes = totalBytes - freeBytes;

        PhysicalStorageDevice dev;
        dev.name = "rootfs";
        dev.devPath = "/dev/root";
        dev.model = "Main System Drive";
        dev.deviceType = "Primary Storage Device";
        dev.badgeIcon = "💾";
        dev.totalGB = totalBytes / (1024.0 * 1024.0 * 1024.0);
        dev.usedGB = usedBytes / (1024.0 * 1024.0 * 1024.0);
        dev.freeGB = freeBytes / (1024.0 * 1024.0 * 1024.0);
        dev.usedPercentage =
            (totalBytes > 0) ? (usedBytes / totalBytes) * 100.0 : 0.0;
        dev.mountPoints.Add("/");
        devices.push_back(dev);
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

  class AudioDevicesPanel : public wxScrolledWindow {
  public:
    AudioDevicesPanel(wxWindow *parent, wxWindowID id = wxID_ANY)
        : wxScrolledWindow(parent, id, wxDefaultPosition, wxDefaultSize,
                           wxVSCROLL) {
      SetScrollRate(0, 10);
      wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);

      wxLogMessage("AudioDevicesPanel constructor called");

      std::vector<AudioDeviceInfo> audioDevices = getAudioDevices();

      wxLogMessage(
          wxString::Format("Found %zu audio devices", audioDevices.size()));

      if (audioDevices.empty()) {
        wxStaticText *errorText = new wxStaticText(
            this, wxID_ANY,
            "No audio devices found or unable to retrieve audio information.");
        mainSizer->Add(errorText, 0, wxALL, 5);
      } else {
        for (const auto &device : audioDevices) {
          wxStaticBoxSizer *deviceSizer =
              new wxStaticBoxSizer(wxVERTICAL, this, device.name);

          wxString typeStr = device.isPlayback ? "Playback" : "Capture";
          deviceSizer->Add(new wxStaticText(this, wxID_ANY, "Type: " + typeStr),
                           0, wxALL, 2);
          deviceSizer->Add(
              new wxStaticText(this, wxID_ANY,
                               "Description: " + device.description),
              0, wxALL, 2);

          wxString formatStr = wxString::Format(
              "Format: %s, %d channels, %d Hz", device.sampleFormat.c_str(),
              device.channels, device.sampleRate);
          deviceSizer->Add(new wxStaticText(this, wxID_ANY, formatStr), 0,
                           wxALL, 2);

          mainSizer->Add(deviceSizer, 0, wxEXPAND | wxALL, 5);

          wxLogMessage(wxString::Format(
              "Added device: %s, Type: %s, Channels: %d, Sample Rate: %d Hz",
              device.name, typeStr, device.channels, device.sampleRate));
        }
      }

      SetSizer(mainSizer);
      FitInside();

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
      m_numCores = 0; // reset before counting

      // Single pass — collect everything from /proc/cpuinfo
      while (std::getline(cpuinfo, line)) {
        std::istringstream iss(line);
        std::string key, value;
        if (std::getline(iss, key, ':') && std::getline(iss, value)) {
          key = Trim(key);
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
      if (physicalCores == 0)
        physicalCores = m_numCores;

      // Architecture
      wxArrayString uname_output;
      wxString arch_str = "Unknown";
      if (wxExecute("uname -m", uname_output, wxEXEC_SYNC) == 0 &&
          !uname_output.IsEmpty()) {
        if (uname_output[0] == "x86_64")
          arch_str = "64-bit";
        else if (uname_output[0].Contains("386") ||
                 uname_output[0].Contains("i686"))
          arch_str = "32-bit";
        else
          arch_str = uname_output[0];
      }

      // ── CPU section ───────────────────────────────────────────────────
      info.emplace_back("CPU Manufacturer", vendor_id);
      info.emplace_back("CPU Model", model_name);
      info.emplace_back("CPU Architecture", arch_str);
      info.emplace_back("CPU Physical Cores", std::to_string(physicalCores));
      info.emplace_back("CPU Logical Threads", std::to_string(m_numCores));

      // ── GPU section ───────────────────────────────────────────────────
      wxString gpuVendor = "Unknown", gpuName = "Unknown",
               gpuDriver = "Unknown", gpuVRAM = "Unknown";

      // lspci — most reliable for name on any GPU
      wxArrayString lspci;
      if (wxExecute("lspci", lspci, wxEXEC_SYNC) == 0) {
        for (const auto &l : lspci) {
          if (l.Contains("VGA") || l.Contains("Display") || l.Contains("3D") ||
              l.Contains("GPU")) {
            gpuName = l.AfterFirst(':').AfterFirst(':').Trim(false);
            break;
          }
        }
      }

      // sysfs — vendor, driver, VRAM
      // Try card0 through card2 explicitly (wxDir glob can miss render nodes)
      for (const wxString &card :
           {wxString("card0"), wxString("card1"), wxString("card2")}) {
        wxString base = "/sys/class/drm/" + card + "/device/";
        if (!wxDirExists(base))
          continue;

        // Vendor
        if (gpuVendor == "Unknown") {
          wxTextFile vf;
          if (vf.Open(base + "vendor")) {
            wxString vid = vf.GetFirstLine().Trim().Lower();
            vf.Close();
            if (vid == "0x1002")
              gpuVendor = "AMD";
            else if (vid == "0x10de")
              gpuVendor = "NVIDIA";
            else if (vid == "0x8086")
              gpuVendor = "Intel";
            else
              gpuVendor = vid;
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

      info.emplace_back("── GPU ──", ""); // divider
      info.emplace_back("GPU Vendor", gpuVendor);
      info.emplace_back("GPU Name", gpuName);
      info.emplace_back("GPU Driver", gpuDriver);
      info.emplace_back("GPU VRAM", gpuVRAM);

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
  miscPageSizer->Add(miscTopRowSizer, 1, wxEXPAND | wxALL, 2);
  miscPageSizer->Add(miscBottomRowSizer, 1, wxEXPAND | wxALL, 2);

  // Set the sizer for the miscInfoPage
  miscInfoPage->SetSizer(miscPageSizer);
  miscInfoPage->FitInside();

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

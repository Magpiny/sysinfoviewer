/**
 * SysInfoViewer Documentation & Showcase Interactive Logic
 * Handles Theme Toggling, Code Copying, Tab Switching, Image Lightbox,
 * and Real-Time Hardware Chart Simulation.
 */

document.addEventListener('DOMContentLoaded', () => {
  initTheme();
  initNavigation();
  initCopyButtons();
  initTabs();
  initLightbox();
  initTelemetrySimulator();
});

/* ==========================================================================
   1. Theme Toggle (Dark / Light)
   ========================================================================== */
function initTheme() {
  const themeToggle = document.getElementById('theme-toggle');
  const prefersDark = window.matchMedia('(prefers-color-scheme: dark)').matches;
  const savedTheme = localStorage.getItem('sysinfoviewer-theme');

  const currentTheme = savedTheme || (prefersDark ? 'dark' : 'dark');
  document.documentElement.setAttribute('data-theme', currentTheme);
  updateThemeIcon(currentTheme);

  if (themeToggle) {
    themeToggle.addEventListener('click', () => {
      const activeTheme = document.documentElement.getAttribute('data-theme');
      const newTheme = activeTheme === 'light' ? 'dark' : 'light';
      document.documentElement.setAttribute('data-theme', newTheme);
      localStorage.setItem('sysinfoviewer-theme', newTheme);
      updateThemeIcon(newTheme);
    });
  }
}

function updateThemeIcon(theme) {
  const themeIcon = document.getElementById('theme-icon');
  if (!themeIcon) return;
  if (theme === 'light') {
    themeIcon.innerHTML = `
      <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
        <path d="M21 12.79A9 9 0 1 1 11.21 3 7 7 0 0 0 21 12.79z"></path>
      </svg>
    `;
  } else {
    themeIcon.innerHTML = `
      <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
        <circle cx="12" cy="12" r="5"></circle>
        <line x1="12" y1="1" x2="12" y2="3"></line>
        <line x1="12" y1="21" x2="12" y2="23"></line>
        <line x1="4.22" y1="4.22" x2="5.64" y2="5.64"></line>
        <line x1="18.36" y1="18.36" x2="19.78" y2="19.78"></line>
        <line x1="1" y1="12" x2="3" y2="12"></line>
        <line x1="21" y1="12" x2="23" y2="12"></line>
        <line x1="4.22" y1="19.78" x2="5.64" y2="18.36"></line>
        <line x1="18.36" y1="5.64" x2="19.78" y2="4.22"></line>
      </svg>
    `;
  }
}

/* ==========================================================================
   2. Sticky Navigation & Smooth Scroll Spy
   ========================================================================== */
function initNavigation() {
  const header = document.querySelector('.header');
  const sections = document.querySelectorAll('section[id]');
  const navLinks = document.querySelectorAll('.nav-link');

  window.addEventListener('scroll', () => {
    if (window.scrollY > 20) {
      header?.classList.add('scrolled');
    } else {
      header?.classList.remove('scrolled');
    }

    let currentSection = '';
    sections.forEach((section) => {
      const sectionTop = section.offsetTop - 120;
      const sectionHeight = section.offsetHeight;
      if (window.scrollY >= sectionTop && window.scrollY < sectionTop + sectionHeight) {
        currentSection = section.getAttribute('id');
      }
    });

    navLinks.forEach((link) => {
      link.classList.remove('active');
      if (link.getAttribute('href') === `#${currentSection}`) {
        link.classList.add('active');
      }
    });
  });
}

/* ==========================================================================
   3. Copy Code Snippet Action
   ========================================================================== */
function initCopyButtons() {
  document.querySelectorAll('.copy-btn').forEach((btn) => {
    btn.addEventListener('click', () => {
      const targetId = btn.getAttribute('data-copy-target');
      const textToCopy = targetId
        ? document.getElementById(targetId)?.innerText
        : btn.previousElementSibling?.innerText || btn.getAttribute('data-clipboard-text');

      if (!textToCopy) return;

      navigator.clipboard.writeText(textToCopy.trim()).then(() => {
        const originalHtml = btn.innerHTML;
        btn.classList.add('copied');
        btn.innerHTML = `
          <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5" stroke-linecap="round" stroke-linejoin="round">
            <polyline points="20 6 9 17 4 12"></polyline>
          </svg> Copied!
        `;

        setTimeout(() => {
          btn.classList.remove('copied');
          btn.innerHTML = originalHtml;
        }, 2200);
      });
    });
  });
}

/* ==========================================================================
   4. Tab Switchers (Showcase, Installation & Hero Mockup)
   ========================================================================== */
function initTabs() {
  // Showcase tabs
  const showcaseTabs = document.querySelectorAll('.showcase-tab');
  const showcasePanels = document.querySelectorAll('.showcase-panel');

  showcaseTabs.forEach((tab) => {
    tab.addEventListener('click', () => {
      const target = tab.getAttribute('data-showcase');
      showcaseTabs.forEach((t) => t.classList.remove('active'));
      showcasePanels.forEach((p) => (p.style.display = 'none'));

      tab.classList.add('active');
      const activePanel = document.getElementById(`showcase-${target}`);
      if (activePanel) activePanel.style.display = 'grid';
    });
  });

  // Installation Hub tabs
  const installTabs = document.querySelectorAll('.install-tab-btn');
  const installPanels = document.querySelectorAll('.install-panel');

  installTabs.forEach((tab) => {
    tab.addEventListener('click', () => {
      const target = tab.getAttribute('data-install-target');
      installTabs.forEach((t) => t.classList.remove('active'));
      installPanels.forEach((p) => p.classList.remove('active'));

      tab.classList.add('active');
      const activePanel = document.getElementById(`install-${target}`);
      if (activePanel) activePanel.classList.add('active');
    });
  });

  // Hero Mockup Header tabs
  const mockupTabBtns = document.querySelectorAll('.mockup-tab-btn');
  const mockupImg = document.getElementById('hero-mockup-img');

  mockupTabBtns.forEach((btn) => {
    btn.addEventListener('click', () => {
      const imgSrc = btn.getAttribute('data-img');
      mockupTabBtns.forEach((b) => b.classList.remove('active'));
      btn.classList.add('active');

      if (mockupImg && imgSrc) {
        mockupImg.style.opacity = '0.3';
        setTimeout(() => {
          mockupImg.src = imgSrc;
          mockupImg.style.opacity = '1';
        }, 150);
      }
    });
  });
}

/* ==========================================================================
   5. Interactive Lightbox Modal
   ========================================================================== */
function initLightbox() {
  const lightbox = document.getElementById('lightbox-modal');
  const lightboxImg = document.getElementById('lightbox-img');
  const lightboxClose = document.getElementById('lightbox-close');

  document.querySelectorAll('[data-lightbox]').forEach((trigger) => {
    trigger.addEventListener('click', (e) => {
      e.preventDefault();
      const imgElement = trigger.querySelector('img') || trigger;
      const fullSrc = trigger.getAttribute('data-lightbox-src') || imgElement.src;

      if (lightbox && lightboxImg && fullSrc) {
        lightboxImg.src = fullSrc;
        lightbox.classList.add('active');
      }
    });
  });

  const closeLightbox = () => {
    if (lightbox) lightbox.classList.remove('active');
  };

  lightboxClose?.addEventListener('click', closeLightbox);
  lightbox?.addEventListener('click', (e) => {
    if (e.target === lightbox) closeLightbox();
  });

  document.addEventListener('keydown', (e) => {
    if (e.key === 'Escape' && lightbox?.classList.contains('active')) {
      closeLightbox();
    }
  });
}

/* ==========================================================================
   6. Live Real-Time Telemetry Simulator (Canvas Doughnut / Pie Charts)
   ========================================================================== */
function initTelemetrySimulator() {
  const cpuCanvas = document.getElementById('sim-cpu-canvas');
  const ramCanvas = document.getElementById('sim-ram-canvas');
  const diskCanvas = document.getElementById('sim-disk-canvas');

  if (!cpuCanvas || !ramCanvas || !diskCanvas) return;

  const cpuCtx = cpuCanvas.getContext('2d');
  const ramCtx = ramCanvas.getContext('2d');
  const diskCtx = diskCanvas.getContext('2d');

  let cpuUsage = 28.5;
  let ramUsedGB = 7.4;
  const ramTotalGB = 16.0;
  const diskUsedGB = 142.6;
  const diskTotalGB = 512.0;

  function resizeCanvas(canvas) {
    const rect = canvas.getBoundingClientRect();
    const dpr = window.devicePixelRatio || 1;
    canvas.width = rect.width * dpr;
    canvas.height = rect.height * dpr;
    const ctx = canvas.getContext('2d');
    ctx.scale(dpr, dpr);
    return { width: rect.width, height: rect.height };
  }

  function drawDoughnut(ctx, width, height, value, total, primaryColor, secondaryColor, innerRatio = 0.72) {
    ctx.clearRect(0, 0, width, height);
    const centerX = width / 2;
    const centerY = height / 2;
    const radius = Math.min(width, height) * 0.42;
    const innerRadius = radius * innerRatio;
    const usedAngle = (2 * Math.PI * value) / total;

    // Track (Unused slice)
    ctx.beginPath();
    ctx.arc(centerX, centerY, radius, usedAngle - Math.PI / 2, 2 * Math.PI - Math.PI / 2);
    ctx.arc(centerX, centerY, innerRadius, 2 * Math.PI - Math.PI / 2, usedAngle - Math.PI / 2, true);
    ctx.closePath();
    ctx.fillStyle = secondaryColor;
    ctx.fill();

    // Active (Used slice)
    ctx.beginPath();
    ctx.arc(centerX, centerY, radius, -Math.PI / 2, usedAngle - Math.PI / 2);
    ctx.arc(centerX, centerY, innerRadius, usedAngle - Math.PI / 2, -Math.PI / 2, true);
    ctx.closePath();
    ctx.fillStyle = primaryColor;
    ctx.fill();
  }

  function drawPie(ctx, width, height, value, total, primaryColor, secondaryColor) {
    ctx.clearRect(0, 0, width, height);
    const centerX = width / 2;
    const centerY = height / 2;
    const radius = Math.min(width, height) * 0.42;
    const usedAngle = (2 * Math.PI * value) / total;

    // Unused Slice (Lavender)
    ctx.beginPath();
    ctx.moveTo(centerX, centerY);
    ctx.arc(centerX, centerY, radius, usedAngle - Math.PI / 2, 2 * Math.PI - Math.PI / 2);
    ctx.closePath();
    ctx.fillStyle = secondaryColor;
    ctx.fill();

    // Used Slice (Light Pink)
    ctx.beginPath();
    ctx.moveTo(centerX, centerY);
    ctx.arc(centerX, centerY, radius, -Math.PI / 2, usedAngle - Math.PI / 2);
    ctx.closePath();
    ctx.fillStyle = primaryColor;
    ctx.fill();
  }

  function renderSim() {
    const cpuSize = resizeCanvas(cpuCanvas);
    const ramSize = resizeCanvas(ramCanvas);
    const diskSize = resizeCanvas(diskCanvas);

    // Render CPU (Light Pink + Sky Blue)
    drawDoughnut(cpuCtx, cpuSize.width, cpuSize.height, cpuUsage, 100, '#ffb6c1', '#87ceeb', 0.72);
    const cpuLabel = document.getElementById('sim-cpu-val');
    if (cpuLabel) cpuLabel.innerText = `${cpuUsage.toFixed(1)}%`;

    // Render RAM (Brown + Green)
    drawDoughnut(ramCtx, ramSize.width, ramSize.height, ramUsedGB, ramTotalGB, '#a52a2a', '#008000', 0.65);
    const ramLabel = document.getElementById('sim-ram-val');
    if (ramLabel) ramLabel.innerText = `${((ramUsedGB / ramTotalGB) * 100).toFixed(1)}%`;

    // Render Disk (Light Pink + Lavender)
    drawPie(diskCtx, diskSize.width, diskSize.height, diskUsedGB, diskTotalGB, '#ffb6c1', '#e6e6fa');
    const diskLabel = document.getElementById('sim-disk-val');
    if (diskLabel) diskLabel.innerText = `${((diskUsedGB / diskTotalGB) * 100).toFixed(1)}%`;
  }

  // Initial draw
  renderSim();

  // Simulate real-time 1.5s refresh fluctuation like Linux kernel stats
  setInterval(() => {
    // Smooth random variation
    const deltaCpu = (Math.random() - 0.48) * 8;
    cpuUsage = Math.min(94, Math.max(8, cpuUsage + deltaCpu));

    const deltaRam = (Math.random() - 0.5) * 0.2;
    ramUsedGB = Math.min(14.5, Math.max(4.2, ramUsedGB + deltaRam));

    renderSim();
  }, 1500);

  window.addEventListener('resize', renderSim);
}

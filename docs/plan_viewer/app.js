// ========== CHECKLIST PERSISTENCE (localStorage) ==========
const STORAGE_KEY = 'hydro_checklist_v1';

function loadChecklist() {
    const saved = localStorage.getItem(STORAGE_KEY);
    if (saved) {
        const data = JSON.parse(saved);
        document.querySelectorAll('.checklist input[type="checkbox"]').forEach(cb => {
            if (data[cb.dataset.task]) cb.checked = true;
        });
    }
    updateProgress();
}

function saveChecklist() {
    const data = {};
    document.querySelectorAll('.checklist input[type="checkbox"]').forEach(cb => {
        data[cb.dataset.task] = cb.checked;
    });
    localStorage.setItem(STORAGE_KEY, JSON.stringify(data));
    showToast('Đã lưu checklist!');
}

function updateProgress() {
    const all = document.querySelectorAll('.checklist input[type="checkbox"]');
    const checked = document.querySelectorAll('.checklist input[type="checkbox"]:checked');
    const total = all.length;
    const done = checked.length;
    const pct = total > 0 ? Math.round((done / total) * 100) : 0;

    // Update sidebar progress ring
    const ring = document.querySelector('.progress-ring');
    if (ring) {
        ring.style.background = `conic-gradient(var(--accent-green) ${pct}%, var(--bg-card) ${pct}%)`;
    }
    const pctEl = document.getElementById('progressPercent');
    if (pctEl) pctEl.textContent = `${pct}%`;

    // Update checklist stats
    const statsEl = document.getElementById('checklistProgress');
    if (statsEl) statsEl.textContent = `${done}/${total} hoàn thành`;
}

// ========== TOAST NOTIFICATION ==========
function showToast(msg) {
    const existing = document.querySelector('.toast');
    if (existing) existing.remove();

    const toast = document.createElement('div');
    toast.className = 'toast';
    toast.innerHTML = `<i class="fas fa-check-circle"></i> ${msg}`;
    toast.style.cssText = `
        position:fixed; bottom:30px; right:30px; z-index:1000;
        background:#1e2130; border:1px solid var(--accent-green);
        color:var(--accent-green); padding:12px 20px;
        border-radius:8px; font-size:14px; font-weight:500;
        display:flex; align-items:center; gap:8px;
        animation: slideIn 0.3s ease, fadeOut 0.3s ease 2s forwards;
        box-shadow: 0 4px 20px rgba(0,0,0,0.4);
    `;
    document.body.appendChild(toast);
    setTimeout(() => toast.remove(), 2500);
}

// Add toast animation
const style = document.createElement('style');
style.textContent = `
    @keyframes slideIn { from { transform:translateY(20px); opacity:0; } to { transform:translateY(0); opacity:1; } }
    @keyframes fadeOut { from { opacity:1; } to { opacity:0; } }
`;
document.head.appendChild(style);

// ========== SIDEBAR NAVIGATION ==========
function setupNavigation() {
    const links = document.querySelectorAll('.nav-link');
    const sections = document.querySelectorAll('.section');

    // Scroll spy
    const observer = new IntersectionObserver(entries => {
        entries.forEach(entry => {
            if (entry.isIntersecting) {
                const id = entry.target.id;
                links.forEach(l => l.classList.remove('active'));
                const active = document.querySelector(`.nav-link[href="#${id}"]`);
                if (active) active.classList.add('active');
            }
        });
    }, { rootMargin: '-20% 0px -70% 0px' });

    sections.forEach(s => observer.observe(s));

    // Smooth scroll
    links.forEach(link => {
        link.addEventListener('click', e => {
            e.preventDefault();
            const target = document.querySelector(link.getAttribute('href'));
            if (target) target.scrollIntoView({ behavior: 'smooth', block: 'start' });
        });
    });
}

// ========== SECTION ANIMATIONS ==========
function setupAnimations() {
    const observer = new IntersectionObserver(entries => {
        entries.forEach(entry => {
            if (entry.isIntersecting) {
                entry.target.style.opacity = '1';
                entry.target.style.transform = 'translateY(0)';
            }
        });
    }, { threshold: 0.1 });

    document.querySelectorAll('.info-card, .checklist-card, .timeline-item, .stat-card, .feasibility-item').forEach(el => {
        el.style.opacity = '0';
        el.style.transform = 'translateY(20px)';
        el.style.transition = 'opacity 0.5s ease, transform 0.5s ease';
        observer.observe(el);
    });
}

// ========== INIT ==========
document.addEventListener('DOMContentLoaded', () => {
    loadChecklist();
    setupNavigation();
    setupAnimations();

    // Checkbox change → update progress
    document.querySelectorAll('.checklist input[type="checkbox"]').forEach(cb => {
        cb.addEventListener('change', () => {
            updateProgress();
            saveChecklist(); // Auto-save on change
        });
    });

    // Save button
    document.getElementById('saveChecklist')?.addEventListener('click', saveChecklist);
});

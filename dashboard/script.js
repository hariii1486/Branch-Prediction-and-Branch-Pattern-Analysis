let charts = {};

const selector = document.getElementById("selector");
selector.addEventListener("change", (e) => loadData(e.target.value));

// Initial Load
loadData(selector.value);

async function loadData(base) {
    try {
        const [h, t] = await Promise.all([
            fetch(`results/${base}.json`).then(r => r.json()),
            fetch(`results/${base}_tage.json`).then(r => r.json())
        ]);
        render(h, t);
    } catch (e) {
        console.error("Data load failed. Check filenames and CORS.", e);
    }
}

function render(h, t) {
    // 1. Update Top Cards
    document.getElementById("total").innerText = h.summary.total;
    const bestAcc = Math.max(h.summary.best_accuracy, t.summary.best_accuracy);
    document.getElementById("best").innerText = bestAcc.toFixed(1) + "%";
    document.getElementById("winner").innerText = t.summary.best_accuracy > h.summary.best_accuracy ? "TAGE wins" : "Hybrid wins";
    document.getElementById("mispred").innerText = h.summary.mispred;
    document.getElementById("warmup").innerText = h.summary.warmup_gain;

    // 2. Main Horizontal Comparison Chart
    const predLabels = ["TAGE \uD83D\uDC51", "Hybrid", "2-bit", "1-bit"];
    const predValues = [t.predictors["TAGE"], h.predictors["Hybrid"], h.predictors["2-bit"], h.predictors["1-bit"]];
    const colors = predValues.map(v => v > 80 ? '#fbc02d' : (v > 70 ? '#f57c00' : '#e53935'));

    updateChart("barChart", "bar", predLabels, [{
        data: predValues,
        backgroundColor: colors,
        borderRadius: 5,
        barThickness: 18
    }], { indexAxis: 'y' });

    // 3. Update Per-Branch Table/List
    const list = document.getElementById("branch-list");
    list.innerHTML = h.branches.map(b => `
        <div class="branch-row">
            <span class="branch-pc">${b.pc}</span>
            <span class="badge ${b.miss < 5 ? 'badge-loop' : 'badge-cond'}">${b.miss < 5 ? 'loop' : 'cond'}</span>
            <span class="branch-code">${b.code || 'if (condition)'}</span>
            <span class="branch-miss">${b.miss}</span>
            <span class="detail-link">detail</span>
        </div>
    `).join("");

    // 4. Mispredictions Chart
    updateChart("missChart", "bar", h.branches.map(b => b.pc), [{
        data: h.branches.map(b => b.miss),
        backgroundColor: h.branches.map(b => b.miss > 15 ? '#f57c00' : '#7cb342')
    }]);

    // 5. GHR Line Chart
    updateChart("ghrChart", "line", ["4 bits", "8 bits", "16 bits", "32 bits"], [{
        data: h.ghr_vs_accuracy,
        borderColor: '#5c92ff',
        tension: 0.4,
        pointBackgroundColor: '#fbc02d'
    }]);

    // 6. Aliasing Chart
    updateChart("aliasChart", "bar", ["TC1", "TC2", "TC3", "TC4", "TC5", "TC6"], [
        { label: 'T1', data: h.aliasing.T1, backgroundColor: '#9575cd' },
        { label: 'T2', data: h.aliasing.T2, backgroundColor: '#4db6ac' }
    ]);
}

function updateChart(id, type, labels, datasets, extra = {}) {
    if (charts[id]) charts[id].destroy();
    const ctx = document.getElementById(id).getContext('2d');
    charts[id] = new Chart(ctx, {
        type: type,
        data: { labels, datasets },
        options: Object.assign({
            responsive: true,
            plugins: { legend: { display: datasets.length > 1, labels: { color: '#999' } } },
            scales: {
                y: { grid: { color: '#3d3d3a' }, ticks: { color: '#999' } },
                x: { grid: { display: false }, ticks: { color: '#999' } }
            }
        }, extra)
    });
}
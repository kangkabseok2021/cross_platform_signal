interface MetricsResponse {
    sorted_count: number;
    avg_confidence: number;
    last_material: string;
}

function updateElementText(id: string, text: string): void {
    const el = document.getElementById(id);
    if (el) {
        el.textContent = text;
    }
}

async function fetchMetrics(): Promise<void> {
    const statusBadge = document.getElementById("status-badge");
    try {
        const response = await fetch("/metrics");
        if (!response.ok) {
            throw new Error("HTTP error " + response.status);
        }
        const data: MetricsResponse = await response.json();
        
        updateElementText("sorted-count", data.sorted_count.toString());
        updateElementText("avg-confidence", data.avg_confidence.toFixed(3));
        updateElementText("last-material", data.last_material);

        if (statusBadge) {
            statusBadge.textContent = "Connected";
            statusBadge.classList.add("connected");
        }
    } catch (error) {
        console.error("Failed to fetch metrics:", error);
        if (statusBadge) {
            statusBadge.textContent = "Disconnected";
            statusBadge.classList.remove("connected");
        }
    }
}

// Poll metrics every 1 second
setInterval(fetchMetrics, 1000);

// Initial load
window.addEventListener("DOMContentLoaded", () => {
    fetchMetrics();
});

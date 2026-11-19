#!/usr/bin/env python3
import argparse
import re
import matplotlib.pyplot as plt

PATTERN = re.compile(r"\[(\d+)\]\s+(LOW|MEDIUM|HIGH)\s+([A-Z0-9_]+)")
CPU_LINE_RE = re.compile(r"^\s*(LOW_ID|MED_ID|HIGH_ID)\s*:.*CPU:\s*(\d+)\s*%")
CYCLES_LINE_RE = re.compile(r"^\s*:\s*Total CPU cycles used:\s*(\d+)")


def parse_logs(log_path):
	events = []
	cpu_snapshots = []
	current_snap = {}
	last_thread = None

	with open(log_path, "r", encoding="utf-8", errors="ignore") as f:
		for line in f:
			m = PATTERN.search(line)
			if m:
				events.append((int(m.group(1)), m.group(2), m.group(3)))

			if "Thread analyze:" in line:
				if current_snap:
					cpu_snapshots.append(current_snap)
					current_snap = {}
				last_thread = None
				continue

			m_cpu = CPU_LINE_RE.match(line)
			if m_cpu:
				th_name = m_cpu.group(1)
				cpu_pct = int(m_cpu.group(2))
				current_snap[th_name] = {"cpu_pct": cpu_pct, "cycles": None}
				last_thread = th_name
				continue

			m_cyc = CYCLES_LINE_RE.match(line)
			if m_cyc and last_thread in current_snap:
				current_snap[last_thread]["cycles"] = int(m_cyc.group(1))
				last_thread = None

	if current_snap:
		cpu_snapshots.append(current_snap)

	latest_cpu = cpu_snapshots[-1] if cpu_snapshots else {}
	return events, latest_cpu


def subtract_intervals(intervals, blockers):
	"""Soustrait une liste d'intervalles bloquants (start, end) à des segments (s, e, meta)."""
	result = []
	for s, e, meta in intervals:
		segs = [(s, e)]
		for bs, be in blockers:
			if be <= s or bs >= e:
				continue
			next_segs = []
			for cs, ce in segs:
				if be <= cs or bs >= ce:
					next_segs.append((cs, ce))
				else:
					if bs > cs:
						next_segs.append((cs, bs))
					if be < ce:
						next_segs.append((be, ce))
			segs = next_segs
		for cs, ce in segs:
			if ce > cs:
				result.append((cs, ce, meta))
	return result


def plot_gantt(events, cpu_stats, out_png="gantt_zoom.png", start_kcycles=0.0, max_kcycles=3000.0):
	if not events:
		print("Aucun événement trouvé.")
		return

	t0 = events[0][0]
	end_kcycles = start_kcycles + max_kcycles
	margin = 500.0

	rel = [
		((c - t0) / 1000.0, th, ev)
		for c, th, ev in events
		if (start_kcycles - margin) <= ((c - t0) / 1000.0) <= end_kcycles
	]

	wait_bars = []
	crit_bars = []
	keep_bars = []
	med_bars = []

	open_wait = {}
	open_crit = {}
	open_keep = {}
	open_med = None

	for t, th, ev in rel:
		if ev == "WAIT_SEM":
			open_wait[th] = t
		elif ev == "GOT_SEM":
			if th in open_wait:
				wait_bars.append((open_wait.pop(th), t, th))
			open_crit[th] = t
		elif ev == "REL_SEM":
			if th in open_crit:
				crit_bars.append((open_crit.pop(th), t, th))
		elif ev == "KEEP_WORKING":
			open_keep[th] = t
		elif ev == "END":
			if th in open_keep:
				keep_bars.append((open_keep.pop(th), t, th))
			elif th in open_crit:
				crit_bars.append((open_crit.pop(th), t, th))
			if th == "MEDIUM" and open_med is not None:
				med_bars.append((open_med, t, "MEDIUM"))
				open_med = None
		elif ev == "START" and th == "MEDIUM":
			open_med = t

	med_blockers = [(s, e) for s, e, _ in med_bars]
	low_crit = [(s, e, th) for s, e, th in crit_bars if th == "LOW"]
	other_crit = [(s, e, th) for s, e, th in crit_bars if th != "LOW"]
	low_crit_natural = subtract_intervals(low_crit, med_blockers) + other_crit

	y_map = {"LOW": 1, "MEDIUM": 2, "HIGH": 3}
	fig, (ax, ax_tbl) = plt.subplots(
		1, 2, figsize=(15.5, 5), gridspec_kw={"width_ratios": [4.2, 1.3]}
	)

	for s, e, th in wait_bars:
		ax.barh(y_map[th], max(e - s, 0.4), left=s, height=0.34, color="#e74c3c", alpha=0.85, label="Attente sémaphore")
	for s, e, th in low_crit_natural:
		ax.barh(y_map[th], max(e - s, 0.4), left=s, height=0.34, color="#2ecc71", alpha=0.9, label="Section critique")
	for s, e, th in keep_bars:
		ax.barh(y_map[th], max(e - s, 0.4), left=s, height=0.34, color="#3498db", alpha=0.85, label="KEEP_WORKING")
	for s, e, _ in med_bars:
		ax.barh(y_map["MEDIUM"], max(e - s, 0.4), left=s, height=0.34, color="#f39c12", alpha=0.9, label="MEDIUM actif")

	ax.set_xlim(start_kcycles, end_kcycles)
	ax.set_yticks([1, 2, 3])
	ax.set_yticklabels(["LOW (prio 7)", "MEDIUM (prio 5)", "HIGH (prio 3)"])
	ax.set_xlabel("Temps relatif depuis le 1er événement (kcycles)")
	ax.set_title(f"Chronologie naturelle [{start_kcycles:.0f} .. {end_kcycles:.0f}] kcycles")
	ax.grid(True, axis="x", linestyle="--", alpha=0.5)

	handles, labels = ax.get_legend_handles_labels()
	unique = dict(zip(labels, handles))
	ax.legend(unique.values(), unique.keys(), loc="upper right", fontsize=8)

	# Panneau stats CPU (thread_analyzer)
	ax_tbl.axis("off")
	rows = []
	for key, label in [("HIGH_ID", "HIGH"), ("MED_ID", "MEDIUM"), ("LOW_ID", "LOW")]:
		info = cpu_stats.get(key)
		if info:
			cyc_str = str(info["cycles"]) if info["cycles"] is not None else "n/a"
			rows.append([label, f"{info['cpu_pct']} %", cyc_str])
		else:
			rows.append([label, "n/a", "n/a"])

	ax_tbl.set_title("Stats CPU (thread_analyzer)", fontsize=10, fontweight="bold", pad=10)
	table = ax_tbl.table(
		cellText=rows,
		colLabels=["Thread", "CPU", "Cycles"],
		loc="center",
		cellLoc="center",
	)
	table.auto_set_font_size(False)
	table.set_fontsize(9)
	table.scale(1.0, 1.8)

	plt.tight_layout()
	plt.savefig(out_png, dpi=180)
	print(f"Gantt enregistré dans : {out_png}")


if __name__ == "__main__":
	parser = argparse.ArgumentParser()
	parser.add_argument("log_path")
	parser.add_argument("--start", type=float, default=0.0)
	parser.add_argument("--window", type=float, default=3000.0)
	parser.add_argument("--out", default="gantt_zoom.png")
	args = parser.parse_args()
	evs, stats = parse_logs(args.log_path)
	plot_gantt(evs, stats, out_png=args.out, start_kcycles=args.start, max_kcycles=args.window)

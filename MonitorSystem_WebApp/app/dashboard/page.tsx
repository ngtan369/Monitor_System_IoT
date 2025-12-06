"use client";

import { useEffect, useState } from "react";
import { ref, onValue, update, set } from "firebase/database";
import { rtdb } from "@/app/lib/firebase-rtdb";
import Navbar from "@/app/components/navbar";

type Sensor = {
  temp: number;
  humi: number;
  light: number;
};

type Control = {
  fan: number;
  led: number;
};

export default function DashboardPage() {
  const [sensor, setSensor] = useState<Sensor | null>(null);
  const [control, setControl] = useState<Control | null>(null);
  const [online, setOnline] = useState<number>(0);
  const [loading, setLoading] = useState(true);
  const [updating, setUpdating] = useState(false);

  useEffect(() => {
    const sysRef = ref(rtdb, "system_1");

    const unsub = onValue(sysRef, (snap) => {
      const val = snap.val();
      if (!val) {
        setSensor(null);
        setControl(null);
        setOnline(0);
        setLoading(false);
        return;
      }
      setSensor(val.sensor ?? null);
      setControl(val.control ?? null);
      setOnline(val.online ?? 0);
      setLoading(false);
    });

    return () => unsub();
  }, []);

  const toggleFan = async () => {
    if (!control) return;
    setUpdating(true);
    try {
      await update(ref(rtdb, "system_1/control"), {
        fan: control.fan ? 0 : 1,
      });
    } finally {
      setUpdating(false);
    }
  };

  const toggleLed = async () => {
    if (!control) return;
    setUpdating(true);
    try {
      await update(ref(rtdb, "system_1/control"), {
        led: control.led ? 0 : 1,
      });
    } finally {
      setUpdating(false);
    }
  };

  // đánh dấu online khi dashboard mở (tuỳ bạn có dùng hay không)
  useEffect(() => {
    const setOnlineFlag = async (v: 0 | 1) => {
      await set(ref(rtdb, "system_1/online"), v);
    };
    setOnlineFlag(1);
    return () => {
      setOnlineFlag(0);
    };
  }, []);

  return (
    <div className=" min-h-screen bg-slate-950 text-slate-50">
      <Navbar />
      <main className="flex-1 p-16 space-y-8 overflow-y-auto">
        <header className="flex items-center justify-between">
          <div>
            <h1 className="text-3xl font-semibold tracking-tight">
              IoT Dashboard – System 1
            </h1>
            <p className="text-sm text-slate-400 mt-1">
              Realtime Database · Giám sát & điều khiển
            </p>
          </div>
          <span
            className={`inline-flex items-center gap-2 rounded-full px-3 py-1 text-xs font-medium ${
              online
                ? "bg-emerald-500/10 text-emerald-400 ring-1 ring-emerald-500/30"
                : "bg-red-500/10 text-red-400 ring-1 ring-red-500/30"
            }`}
          >
            <span className="h-2 w-2 rounded-full bg-current animate-pulse" />
            {online ? "Device online" : "Device offline"}
          </span>
        </header>

        {loading ? (
          <div className="flex h-40 items-center justify-center">
            <div className="h-6 w-6 animate-spin rounded-full border-2 border-slate-500 border-t-transparent" />
          </div>
        ) : (
          <div className="grid gap-6 md:grid-cols-3">
            <SensorCard sensor={sensor} />
            <ControlCard
              control={control}
              updating={updating}
              toggleFan={toggleFan}
              toggleLed={toggleLed}
            />
          </div>
        )}
      </main>
    </div>
  );
}

function SensorCard({ sensor }: { sensor: Sensor | null }) {
  if (!sensor) {
    return (
      <section className="col-span-2 rounded-2xl border border-slate-800 bg-slate-900/60 p-6">
        <h2 className="text-lg font-semibold mb-2">Sensor Realtime</h2>
        <p className="text-sm text-slate-500">
          Chưa có dữ liệu cảm biến cho system_1.
        </p>
      </section>
    );
  }
  return (
    <section className="col-span-2 rounded-2xl border border-slate-800 bg-slate-900/60 p-6 shadow-lg shadow-black/40">
      <div className="flex items-center justify-between mb-4">
        <h2 className="text-lg font-semibold">Sensor Realtime</h2>
        <span className="text-xs text-slate-500">
          RTDB: <code className="font-mono">system_1/sensor</code>
        </span>
      </div>
      <div className="grid grid-cols-3 gap-4">
        <MetricCard
          label="Temperature"
          value={sensor.temp.toFixed(1)}
          unit="°C"
          gradient="from-amber-400/80 to-orange-500/80"
        />
        <MetricCard
          label="Humidity"
          value={sensor.humi.toFixed(1)}
          unit="%"
          gradient="from-sky-400/80 to-cyan-500/80"
        />
        <MetricCard
          label="Light"
          value={sensor.light.toFixed(0)}
          unit="%"
          gradient="from-violet-400/80 to-fuchsia-500/80"
        />
      </div>
    </section>
  );
}

function ControlCard({
  control,
  updating,
  toggleFan,
  toggleLed,
}: {
  control: Control | null;
  updating: boolean;
  toggleFan: () => void;
  toggleLed: () => void;
}) {
  return (
    <section className="rounded-2xl border border-slate-800 bg-slate-900/60 p-6 shadow-lg shadow-black/40">
      <div className="flex items-center justify-between mb-4">
        <h2 className="text-lg font-semibold">Control</h2>
        <span className="text-xs text-slate-500">
          RTDB: <code className="font-mono">system_1/control</code>
        </span>
      </div>
      <div className="space-y-4">
        <ToggleRow
          label="Fan"
          description="Quạt làm mát hệ thống."
          on={!!control?.fan}
          loading={updating}
          onClick={toggleFan}
        />
        <ToggleRow
          label="LED"
          description="Đèn trạng thái."
          on={!!control?.led}
          loading={updating}
          onClick={toggleLed}
        />
      </div>
    </section>
  );
}

function MetricCard(props: {
  label: string;
  value: string;
  unit?: string;
  gradient: string;
}) {
  const { label, value, unit, gradient } = props;
  return (
    <div className="rounded-xl border border-slate-800 bg-gradient-to-br from-slate-900 to-slate-950 p-4">
      <p className="text-xs uppercase tracking-wide text-slate-400">
        {label}
      </p>
      <div className="mt-2 flex items-end gap-1">
        <span
          className={`bg-clip-text text-3xl font-semibold text-transparent bg-gradient-to-r ${gradient}`}
        >
          {value}
        </span>
        {unit && (
          <span className="text-sm text-slate-500 mb-1">{unit}</span>
        )}
      </div>
    </div>
  );
}

function ToggleRow(props: {
  label: string;
  description: string;
  on: boolean;
  loading: boolean;
  onClick: () => void;
}) {
  const { label, description, on, loading, onClick } = props;
  return (
    <div className="flex items-center justify-between gap-3 rounded-xl border border-slate-800 bg-slate-950/60 px-4 py-3">
      <div>
        <p className="text-sm font-medium">{label}</p>
        <p className="text-xs text-slate-500">{description}</p>
      </div>
      <button
        onClick={onClick}
        disabled={loading}
        className={`relative inline-flex h-7 w-12 items-center rounded-full border transition ${
          on
            ? "border-emerald-400 bg-emerald-500/20"
            : "border-slate-600 bg-slate-800"
        } ${loading ? "opacity-60 cursor-wait" : "cursor-pointer"}`}
      >
        <span
          className={`inline-block h-5 w-5 transform rounded-full bg-white shadow transition ${
            on ? "translate-x-6" : "translate-x-1"
          }`}
        />
      </button>
    </div>
  );
}

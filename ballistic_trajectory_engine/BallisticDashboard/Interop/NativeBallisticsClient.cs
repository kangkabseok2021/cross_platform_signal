using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using BallisticDashboard.Models;

namespace BallisticDashboard.Interop;

public sealed class BallisticsException : Exception
{
    public BallisticsException(string message) : base(message) { }
}

public sealed class NativeBallisticsClient
{
    private const int MaxPoints = 50_000;

    public IReadOnlyList<TrajectoryPoint> Compute(
        double elevationDeg,
        double muzzleVelocityMs,
        MunitionKind munition)
    {
        var buffer = new TrajectoryPointNative[MaxPoints];
        int count;
        int result = NativeBindings.compute_trajectory(
            elevationDeg, muzzleVelocityMs, (int)munition,
            buffer, MaxPoints, out count);

        if (result < 0)
        {
            string err = Marshal.PtrToStringAnsi(NativeBindings.get_last_error())
                         ?? "Unknown native error";
            throw new BallisticsException(err);
        }

        return buffer.Take(count)
                     .Select(p => new TrajectoryPoint(p.TimeS, p.XMeters, p.YMeters, p.SpeedMs))
                     .ToList();
    }
}

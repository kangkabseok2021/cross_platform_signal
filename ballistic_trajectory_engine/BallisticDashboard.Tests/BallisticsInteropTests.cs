using System;
using System.Linq;
using System.Runtime.InteropServices;
using Xunit;

namespace BallisticDashboard.Tests;

// P/Invoke declarations mirrored here so the test project has no WPF dependency.
[StructLayout(LayoutKind.Sequential)]
file struct TrajectoryPointNative
{
    public double TimeS, XMeters, YMeters, SpeedMs;
}

file static class Native
{
    static Native()
    {
        var path = Environment.GetEnvironmentVariable("BALLISTIC_DLL_PATH");
        if (!string.IsNullOrEmpty(path))
            NativeLibrary.Load(path);
    }

    [DllImport("ballistic_engine", CallingConvention = CallingConvention.Cdecl)]
    public static extern int compute_trajectory(
        double elevation_deg, double muzzle_velocity_ms, int munition_id,
        [Out, MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 4)]
        TrajectoryPointNative[] out_points, int max_points, out int out_count);

    [DllImport("ballistic_engine", CallingConvention = CallingConvention.Cdecl)]
    public static extern IntPtr get_last_error();
}

[Trait("Category", "Interop")]
public sealed class BallisticsInteropTests
{
    private static readonly bool DllAvailable =
        !string.IsNullOrEmpty(Environment.GetEnvironmentVariable("BALLISTIC_DLL_PATH"));

    private static (TrajectoryPointNative[] pts, int count) Compute(
        double elev, double vel, int munId)
    {
        var buf = new TrajectoryPointNative[50_000];
        int n;
        int rc = Native.compute_trajectory(elev, vel, munId, buf, buf.Length, out n);
        return (buf, rc < 0 ? -1 : n);
    }

    [SkippableFact]
    public void ComputeArtillery155_45Deg_ReturnsNonEmptyList()
    {
        Skip.IfNot(DllAvailable, "Native DLL not available");
        var (pts, count) = Compute(45.0, 400.0, 0);
        Assert.True(count > 0);
        Assert.InRange(pts[count - 1].YMeters, -0.5, 0.5); // lands at ground
    }

    [SkippableFact]
    public void ComputeWithZeroElevation_ReturnsError()
    {
        Skip.IfNot(DllAvailable, "Native DLL not available");
        var (_, count) = Compute(0.0, 400.0, 0);
        Assert.Equal(-1, count);
    }

    [SkippableFact]
    public void ComputeWithNegativeVelocity_ReturnsError()
    {
        Skip.IfNot(DllAvailable, "Native DLL not available");
        var (_, count) = Compute(45.0, -100.0, 0);
        Assert.Equal(-1, count);
    }

    [SkippableTheory]
    [InlineData(0)]
    [InlineData(1)]
    [InlineData(2)]
    public void AllMunitionTypes_ReturnResults(int munitionId)
    {
        Skip.IfNot(DllAvailable, "Native DLL not available");
        var (_, count) = Compute(45.0, 400.0, munitionId);
        Assert.True(count > 0, $"munition_id={munitionId} returned no points");
    }

    [SkippableFact]
    public void MaxAltitude_AtHighElevation_GreaterThan_LowElevation()
    {
        Skip.IfNot(DllAvailable, "Native DLL not available");
        var (pts_lo, cnt_lo) = Compute(10.0, 400.0, 0);
        var (pts_hi, cnt_hi) = Compute(80.0, 400.0, 0);
        Assert.True(cnt_lo > 0 && cnt_hi > 0);
        double apex_lo = pts_lo.Take(cnt_lo).Max(p => p.YMeters);
        double apex_hi = pts_hi.Take(cnt_hi).Max(p => p.YMeters);
        Assert.True(apex_hi > apex_lo,
            $"Expected 80° apex ({apex_hi:F0} m) > 10° apex ({apex_lo:F0} m)");
    }
}

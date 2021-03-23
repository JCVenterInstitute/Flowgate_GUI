package flowgate.transform;

public class LogicleTransform extends Transform {

    int TAYLOR_SERIES_LENGTH = 16;
    double EPSILON = 2.220446049250313e-16;

    double T;
    double A;
    double M;
    double W;

    // Internal values -------------------------------------------------
    double a;
    double b;
    double c;
    double d;
    double f;
    double w;
    double x0;
    double x1;
    double x2;
    double xTaylor;
    double taylorSeries[] = new double[TAYLOR_SERIES_LENGTH];

    LogicleTransform() {
        this(262144.0, 0.0, 4.5, 1.5);
    }

    LogicleTransform(double t, double a, double m, double w) {
        this.T = t;
        this.A = a;
        this.M = m;
        this.W = w;

        initialize();
    }

    public double getT() {
        return T;
    }

    public void setT(double t) {
        T = t;
    }

    public double getA() {
        return A;
    }

    public void setA(double a) {
        A = a;
    }

    public double getM() {
        return M;
    }

    public void setM(double m) {
        M = m;
    }

    public double getW() {
        return W;
    }

    public void setW(double w) {
        W = w;
    }

    void initialize() {
        // Use the transform's (T,A,M,W) values and compute intermediate
        // values.
        this.w = W / (M + A);
        this.x2 = A / (M + A);
        this.x1 = this.x2 * this.w;
        this.x0 = this.x1 * 2.0;
        this.b = (M + A) * Math.log(10.0);

        this.d = this.solve(this.b, this.w);

        double exp_b_x1 = Math.exp(this.b * this.x1);
        double exp_d_x1 = Math.exp(this.d * this.x1);

        double c_a = Math.exp(this.x0 * (this.b + this.d));
        double mf_a = exp_b_x1 - c_a / exp_d_x1;

        this.a = this.T /
                ((Math.exp(this.b) - mf_a) - c_a / Math.exp(this.d));
        this.c = c_a * this.a;
        this.f = -mf_a * this.a;

        // To prevent round off problems from the formal definition,
        // use a Taylor series near x1.
        this.xTaylor = this.x1 + this.w / 4.0;

        double posCoef = this.a * exp_b_x1;
        double negCoef = -this.c * exp_d_x1;
        for (int i = 0; i < this.TAYLOR_SERIES_LENGTH; ++i) {
            posCoef *= (this.b / (double) (i + 1));
            negCoef *= -(this.d / (double) (i + 1));
            this.taylorSeries[i] = posCoef * negCoef;
        }

        // Force the 1-st entry to zero as the exact result of the
        // logicle condition.
        this.taylorSeries[1] = 0.0;
    }

    double seriesBiexponential(double scale) {
        double x = scale - this.x1;
        double sum = taylorSeries[TAYLOR_SERIES_LENGTH - 1] * x;

        for (int i = TAYLOR_SERIES_LENGTH - 2; i >= 2; --i)
            sum = (sum + this.taylorSeries[i]) * x;

        return (sum * x + this.taylorSeries[0]) * x;
    }

    double solve(double b, double w) {
        // When w == 0, the logicle function is really an inverse
        // hyperbolic sine.
        if (w == 0.0)
            return b;

        // Compute the tolerance.
        double tolerance = 2.0 * b * this.EPSILON;

        // Use "RTSAFE" from the book "Numerical Recipes".
        // Bracket the root.
        double d_lo = 0.0;
        double d_hi = b;

        // Bisection first step.
        double d = (d_lo + d_hi) / 2.0;
        double last_delta = d_hi - d_lo;
        double delta = 0.0;

        // Evaluate the function f(w,b) = 2 * (ln(d) - ln(b)) + w * (b + d)
        // and its derivative.
        double f_b = -2.0 * Math.log(b) + w * b;
        double f = 2.0 * Math.log(d) + w * d + f_b;
        double last_f = Double.NaN;

        for (int i = 1; i < 20; ++i) {
            // Compute the derivative.
            double df = (2.0 / d) + w;

            // If Newton's method would step outside the bracket
            // or if it isn't converging quickly enough.
            if ((((d - d_hi) * df - f) * ((d - d_lo) * df - f) >= 0.0) ||
                    (Math.abs(1.9 * f) > Math.abs(last_delta * df))) {
                // Take a bisection step.
                delta = (d_hi - d_lo) / 2.0;
                d = d_lo + delta;
                if (d == d_lo)
                    return d;           // Nothing changed, we're done.
            } else {
                // Otherwise take a Newton's method step.
                delta = f / df;
                double t = d;
                d -= delta;
                if (d == t)
                    return d;           // Nothing changed, we're done.
            }

            // If we've reached the desired precision, we're done.
            if (Math.abs(delta) < tolerance)
                return d;
            last_delta = delta;

            // Recompute the function.
            f = 2.0 * Math.log(d) + w * d + f_b;
            if (f == 0.0 || f == last_f)
                return d;               // Found root or not going to get closer
            last_f = f;

            // Update the bracketing interval.
            if (f < 0.0)
                d_lo = d;
            else
                d_hi = d;
        }

        return d;
    }

    double transformInternal(double value) {
        // Handle true zero specially.
        if (value == 0.0)
            return this.x1;

        // Reflect negative values.
        boolean wasNegative = value < 0.0;
        if (wasNegative == true)
            value = -value;

        // Make an initial guess at the solution.
        double x;
        if (value < this.f) {
            // Use linear approximation in the quasi-linear region.
            x = this.x1 + value / this.taylorSeries[0];
        } else {
            // Otherwise use an ordinary natural log.
            x = Math.log(value / this.a) / this.b;
        }

        // Try for double precision unless in the extended range.
        double tolerance = 3.0 * this.EPSILON;
        if (x > 1.0)
            tolerance = 3.0 * x * this.EPSILON;

        for (int i = 0; i < 10; ++i) {
            // Compute the function and its first two derivatives.
            double ae2bx = this.a * Math.exp(this.b * x);
            double ce2mdx = this.c / Math.exp(this.d * x);
            double y;
            if (x < this.xTaylor)
                // near zero use the Taylor series
                y = this.seriesBiexponential(x) - value;
            else
                // This formulation has better roundoff behavior
                y = (ae2bx + this.f) - (ce2mdx + value);

            double abe2bx = this.b * ae2bx;
            double cde2mdx = this.d * ce2mdx;
            double dy = abe2bx + cde2mdx;
            double ddy = this.b * abe2bx - this.d * cde2mdx;

            // This is Halley's method with cubic convergence.
            double delta = y / (dy * (1.0 - y * ddy / (2.0 * dy * dy)));
            x -= delta;

            // If we've reached the desired precision, we're done.
            if (Math.abs(delta) < tolerance) {
                // Restore the original negative, if needed.
                if (wasNegative == true)
                    return 2.0 * this.x1 - x;
                return x;
            }
        }

        // Did not converge? Close enough?
        return x;
    }

    @Override
    public String toString() {
        return "LogicleTransform{" +
                "T=" + T +
                ", A=" + A +
                ", M=" + M +
                ", W=" + W +
                '}';
    }
}

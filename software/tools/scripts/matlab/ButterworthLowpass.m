classdef ButterworthLowpass

    properties (Access = private)
        m_cutoffFrequency
        m_order

        m_zeros
        m_poles
        m_gain

        m_nominator
        m_denominator
    end

    methods (Access = private)

        function capacitances = calculateCapacitances(self, a, b, resistances)
            capacitances(1) = a / (resistances(1) + resistances(2));
            capacitances(2) = b / (capacitances(1) * resistances(1) * resistances(2));
        end

    end

    methods (Access = public)

        function self = ButterworthLowpass(cutoffFrequency, order)
            self.m_cutoffFrequency = cutoffFrequency;

            if (mod(order, 2) ~= 0)
                display("Odd order, adding 1")
                order = order + 1;
            end

            self.m_order = order;

            omega = 2 * pi * self.m_cutoffFrequency;
            [self.m_zeros, self.m_poles, self.m_gain] = butter(self.m_order, omega, "s");
            [self.m_nominator, self.m_denominator] = zp2tf(self.m_zeros, self.m_poles, self.m_gain);

        end

        function plotFrequencyResponse(self, numberOfSamples, path)
            [H, omega] = freqs(self.m_nominator, self.m_denominator, numberOfSamples);
            fig = figure("visible", "off");
            plot(omega / (2 * 1e3 * pi), mag2db(abs(H)));
            axis([0 2 -40 5]);
            grid;
            xlabel("Frequency (kHz)");
            ylabel("Attenuation (dB)");
            title("Frequency response");
            saveas(fig, path);
        end

        function capacitances = getSellenKeyCapacitances(self, resistances)

            i = 1;
            j = 1;

            while i < self.m_order
                poles(j, :) = [self.m_poles(i), self.m_poles(i + 1)];
                i = i + 2;
                j = j + 1;
            end

            for i = 1:self.m_order / 2
                polynomials(i, :) = poly(poles(i, :));
                polynomials(i, :) = polynomials(i, :) ./ polynomials(i, 3);

                b = polynomials(i, 1);
                a = polynomials(i, 2);
                capacitances(i, :) = self.calculateCapacitances(a, b, resistances(i, :));

            end

        end

    end

end

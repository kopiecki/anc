function design_filter(cutoffFrequency, order, plot_path, caps_path)
    NUMBER_OF_SAMPLES = 4096;

    butterworthLowpass = ButterworthLowpass(cutoffFrequency, order);
    butterworthLowpass.plotFrequencyResponse(NUMBER_OF_SAMPLES, plot_path);

    R = [47e+3, 100e+3
        47e+3, 120e+3
        47e+3, 140e+3];

    caps = butterworthLowpass.getSellenKeyCapacitances(R);
    save(caps_path, 'caps', '-ascii');
end

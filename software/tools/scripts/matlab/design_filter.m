function design_filter(cutoffFrequency, order, plot_path, caps_path)
    NUMBER_OF_SAMPLES = 4096;

    butterworthLowpass = ButterworthLowpass(cutoffFrequency, order);
    butterworthLowpass.plotFrequencyResponse(NUMBER_OF_SAMPLES, plot_path);

    C = [4.7e-9, 5.6e-9
        2.7e-9, 6.8e-9
        560e-12, 22e-9];

    resistances = butterworthLowpass.getSellenKeyParameters(butterworthLowpass.RES, C)
    save(caps_path, 'resistances', '-ascii');
end

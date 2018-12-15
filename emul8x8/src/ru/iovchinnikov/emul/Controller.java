package ru.iovchinnikov.emul;

public class Controller {
    private long prevTime = System.currentTimeMillis();
    private double delayCounter = 0;
    private double delay = 300;
    private boolean act = false;
    RU_alph alph = new RU_alph();

    private final Window win;
    Controller(Window win) {
        this.win = win;
    }

    void onPanelRepaint() {
        if (!act) return;
        double deltaTime = (System.currentTimeMillis() - prevTime) / 1000f;
        delayCounter += deltaTime;
        if (delayCounter > delay / 1000) {
            onDelay();
            delayCounter = 0;
        }
        prevTime = System.currentTimeMillis();
    }

    short[] codes;

    private int step;
    private void moveAB(short[] str) {
        if (step < codes.length - 1) {
            for (int i = 0; i < 8; i++) {
                if (step - i <= 0) {
                    win.updPan(step + 1, 0x00);
                } else {
                    win.updPan(8 - i, str[step - i]);
                }
            }
            step++;
        } else {
            step = 0;
        }
    }

    private void onDelay() {
        moveAB(codes);
    }

    public void setStr(String str) {
        codes = new short[str.length() * 6 + 15];
        for (int i = 0; i < str.length(); i++) {
            int letter = (int) str.charAt(i);
            if (letter == 1025) // Ё
                System.arraycopy(alph.YO, 0, codes, ((i + 1) * 6), 6);
            else
                System.arraycopy(alph.LETTERS[letter - 1040], 0, codes, ((i + 1) * 6), 6);
        }
    }

    public void setAct(boolean act) {
        this.act = act;
    }

    public void setDelay(double delay) {
        this.delay = delay;
    }

}

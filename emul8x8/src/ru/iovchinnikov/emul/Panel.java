package ru.iovchinnikov.emul;

import javax.swing.*;
import java.awt.*;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;

public class Panel extends JPanel {
    private static final int OVAL_SIZE = 45;
    private static final boolean[][] leds = new boolean[8][8];

    private final Window main;
    private final Controller controller;

    Panel(Window main, Controller controller) {
        setBackground(Color.BLACK);
        this.main = main;
        this.controller = controller;
        addMouseListener(new MouseAdapter() {
            @Override
            public void mouseReleased(MouseEvent e) {
                super.mouseReleased(e);
                onMouseClicked(e);
            }
        });
    }

    @Override
    protected void paintComponent(Graphics g) {
        super.paintComponent(g);
        render(g);
    }

    private void render(Graphics g) {
        for (int i = 0; i < leds.length; i++) {
            for (int j = 0; j < leds[i].length; j++) {
                g.setColor(leds[i][j] ? Color.RED : Color.DARK_GRAY);
                g.fillOval(i * OVAL_SIZE + 1, j * OVAL_SIZE + 1 , OVAL_SIZE - 2, OVAL_SIZE - 2);
            }
        }
        try {
            Thread.sleep(17);
        } catch (InterruptedException e) {
            throw new RuntimeException(e);
        }
        controller.onPanelRepaint();
        repaint();
    }

    private void onMouseClicked(MouseEvent e) {
        int x = e.getX() / (getWidth() / leds.length);
        int y = e.getY() / (getHeight() / leds[0].length);
        leds[x][y] = !leds[x][y];
        repaint();
        main.setText(x + 1, getValue(x));
    }

    private int getValue(int col) {
        int result = 0;
        for (int i = 0; i < leds[col].length; i++) {
            result += (leds[col][i] ? (1 << i) : 0);
        }
        return result;
    }

    void upd() {
        for (int i = 0; i < leds.length; i++) {
            for (int j = 0; j < leds[i].length; j++) {
                leds[i][j] = false;
            }
        }
        repaint();
    }

    void upd(String value) {
        value = value.substring(2, value.length() - 3);
        String[] a = value.split(", ");
        for (int i = 0; i < a.length; i++) {
            int v = Integer.parseInt(a[i].substring(2), 16);
            for (int j = 0; j < 8; j++) {
                leds[i][j] = (v % 2) != 0;
                v >>= 1;
            }
        }
        repaint();
    }

    void upd(int col, int val) {
        for (int j = 0; j < 8; j++) {
            leds[col - 1][j] = (val % 2) != 0;
            val >>= 1;
        }
        repaint();
    }

    void upd(int col, String val) {
        int v = Integer.parseInt(val.substring(2), 16);
        for (int j = 0; j < 8; j++) {
            leds[col - 1][j] = (v % 2) != 0;
            v >>= 1;
        }
        repaint();
    }

}

package ru.iovchinnikov.emul;

import javax.swing.*;
import java.awt.*;
import java.awt.datatransfer.Clipboard;
import java.awt.datatransfer.StringSelection;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

public class Window extends JFrame implements ActionListener {

    private JTextField[] tfArr = new JTextField[8];
    private JPanel pTF = new JPanel(new GridBagLayout());
    private Panel pan;
    private Controller ctrl;
    private GridBagConstraints c = new GridBagConstraints();
    private JButton btnClr = new JButton("clr");
    private JButton btnDecode = new JButton("dcd");
    private JButton btnText = new JButton("txt");
    private void initTF() {
        for (int i = 0; i < 8; i++) {
            tfArr[i] = new JTextField();
            tfArr[i].setColumns(3);
            c.fill = GridBagConstraints.HORIZONTAL;
            c.gridx = i;
            c.gridy = 0;
            pTF.add(tfArr[i]);
        }
    }

    Window() {
        setDefaultCloseOperation(WindowConstants.EXIT_ON_CLOSE);
        setResizable(false);
        setTitle("Fancy 8x8 editor");
        setBounds(100, 100, 370, 450);
        ctrl = new Controller(this);
        pan = new Panel(this, ctrl);

        initTF();
        btnClr.addActionListener(this);
        btnDecode.addActionListener(this);
        btnText.addActionListener(this);

        c.gridy = 1;
        c.gridx = 0;
        pTF.add(new JLabel(" "));
        c.gridx = 1;
        pTF.add(new JLabel(" "));
        c.gridx = 2;
        pTF.add(new JLabel(" "));
        c.gridx = 3;
        pTF.add(new JLabel(" "));
        c.gridx = 4;
        pTF.add(new JLabel(" "));
        c.gridx = 5;
        pTF.add(btnClr, c);
        c.gridx = 6;
        pTF.add(btnDecode, c);
        c.gridx = 7;
        pTF.add(btnText, c);
        add(pan, BorderLayout.CENTER);
        add(pTF, BorderLayout.SOUTH);
        setVisible(true);
    }

    void setText(int col, int number) {
        tfArr[col - 1].setText(String.format("0x%h", number));
        StringBuilder sb = new StringBuilder("{ ");
        for (int i = 0; i < tfArr.length; i++) {
            sb.append("".equals(tfArr[i].getText()) ? "0x00" : tfArr[i].getText());
            if (i < 7) sb.append(", ");
        }
        sb.append(" };");
        StringSelection stringSelection = new StringSelection(sb.toString());
        Clipboard clip = Toolkit.getDefaultToolkit().getSystemClipboard();
        clip.setContents(stringSelection, null);
        pTF.repaint();
    }

    void updPan(int col, int number) {
        pan.upd(col, number);
    }

    void updPan(int col, String number) {
        pan.upd(col, number);
    }

    @Override
    public void actionPerformed(ActionEvent e) {
        Object src = e.getSource();
        if (src == btnClr) {
            ctrl.setAct(false);
            clearAll();
        } else if (src == btnDecode) {
            ctrl.setAct(false);
            String val = JOptionPane.showInputDialog(this, "Input a 8-length array like {___,___,...};");
            clearAll();
            pan.upd(val);
            val = val.substring(2, val.length() - 3);
            String[] a = val.split(", ");
            for (int i = 0; i < tfArr.length; i++) {
                tfArr[i].setText(a[i]);
            }
        } else if (src == btnText) {
            String val = JOptionPane.showInputDialog(this, "Input a string to show CYRILLIC CAPS, no spaces");
            //ctrl.setStr("АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЬЫЭЮЯабвгдеёжзийклмнопрстуфхцчшщэюя 0123456789,.!№;%:?*()_+@#$%^&*=");
            //ctrl.setStr("АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЬЫЭЮЯ");
            ctrl.setStr(val);
            ctrl.setAct(true);
        }
    }

    private void clearAll() {
        pan.upd();
        for (int i = 0; i < tfArr.length; i++)
            tfArr[i].setText("");
    }
}

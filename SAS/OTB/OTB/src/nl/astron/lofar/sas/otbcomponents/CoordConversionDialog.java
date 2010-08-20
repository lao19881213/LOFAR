/* TreeInfoDialog.java
 *
 *  Copyright (C) 2002-2007
 *  ASTRON (Netherlands Foundation for Research in Astronomy)
 *  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

package nl.astron.lofar.sas.otbcomponents;
import java.awt.Color;
import nl.astron.lofar.lofarutils.LofarUtils;
import nl.astron.lofar.sas.otb.MainFrame;
import org.apache.log4j.Logger;


/**
 *
 * @created 24-03-2006, 12:34
 *
 * @author  coolen
 *
 * @version $Id: TreeInfoDialog.java 16103 2010-07-30 11:01:55Z coolen $
 */
public class CoordConversionDialog extends javax.swing.JDialog {
    static Logger logger = Logger.getLogger(CoordConversionDialog.class);
    static String name = "TreeInfoDialog";
    
    /** Creates new form BeanForm
     * 
     * @param   parent      Frame where this dialog belongs
     * @param   modal       Should the Dialog be modal or not
     * @param   aTree       the Tree we work with
     * @param   aMainFrame  the Mainframe we are part off
     */
    public CoordConversionDialog(boolean modal, MainFrame aMainFrame) {
        super(aMainFrame, modal);
        initComponents();
        itsMainFrame = aMainFrame;

        init();        
    }

    
  
    private void init() {


        isInitialized=false;
        topLabel.setText("Coordinate Conversion Tool");

        isInitialized=true;
 
    }
  
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        topLabel = new javax.swing.JTextArea();
        cancelButton = new javax.swing.JButton();
        jLabel1 = new javax.swing.JLabel();
        jLabel6 = new javax.swing.JLabel();
        radianInput = new javax.swing.JTextField();
        jLabel3 = new javax.swing.JLabel();
        minuteInput = new javax.swing.JTextField();
        jLabel10 = new javax.swing.JLabel();
        degreesInput = new javax.swing.JTextField();
        clearFieldsButton = new javax.swing.JButton();
        degreesDMSInput = new javax.swing.JTextField();
        momIDLabel1 = new javax.swing.JLabel();
        secondInput = new javax.swing.JTextField();
        jLabel2 = new javax.swing.JLabel();
        convertFromInput = new javax.swing.JComboBox();
        jLabel4 = new javax.swing.JLabel();
        hourInput = new javax.swing.JTextField();
        jLabel5 = new javax.swing.JLabel();
        minuteDMSInput = new javax.swing.JTextField();
        momIDLabel2 = new javax.swing.JLabel();
        secondDMSInput = new javax.swing.JTextField();
        jLabel7 = new javax.swing.JLabel();
        jLabel8 = new javax.swing.JLabel();

        setDefaultCloseOperation(javax.swing.WindowConstants.DISPOSE_ON_CLOSE);
        setTitle("Coordinate Conversions");
        setAlwaysOnTop(true);
        setModal(true);
        setName("loadFileDialog"); // NOI18N
        setResizable(false);
        getContentPane().setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        topLabel.setColumns(20);
        topLabel.setEditable(false);
        topLabel.setFont(new java.awt.Font("Tahoma", 1, 11));
        topLabel.setRows(2);
        topLabel.setText("Coordinate Conversions");
        topLabel.setOpaque(false);
        getContentPane().add(topLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(139, 0, 154, 21));

        cancelButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_cancel.png"))); // NOI18N
        cancelButton.setText("Cancel");
        cancelButton.setToolTipText("Cancel filesearch");
        cancelButton.setHorizontalAlignment(javax.swing.SwingConstants.LEFT);
        cancelButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                cancelButtonActionPerformed(evt);
            }
        });
        getContentPane().add(cancelButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 460, 100, -1));

        jLabel1.setText("Hours:");
        getContentPane().add(jLabel1, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 50, -1, 20));

        jLabel6.setText("Radians:");
        getContentPane().add(jLabel6, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 270, -1, 20));

        radianInput.setToolTipText("Radians");
        getContentPane().add(radianInput, new org.netbeans.lib.awtextra.AbsoluteConstraints(100, 270, 200, -1));

        jLabel3.setText("Minutes:");
        getContentPane().add(jLabel3, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 80, -1, 20));

        minuteInput.setToolTipText("Minutes in HMS notation");
        getContentPane().add(minuteInput, new org.netbeans.lib.awtextra.AbsoluteConstraints(100, 80, 200, -1));

        jLabel10.setText("Degrees:");
        getContentPane().add(jLabel10, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 310, -1, 20));

        degreesInput.setToolTipText("Degrees");
        getContentPane().add(degreesInput, new org.netbeans.lib.awtextra.AbsoluteConstraints(100, 310, 200, -1));

        clearFieldsButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_undo.png"))); // NOI18N
        clearFieldsButton.setText("Clear Fields");
        clearFieldsButton.setHorizontalAlignment(javax.swing.SwingConstants.LEFT);
        clearFieldsButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                clearFieldsButtonActionPerformed(evt);
            }
        });
        getContentPane().add(clearFieldsButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(120, 460, 130, -1));

        degreesDMSInput.setToolTipText("Hours in HMS notation");
        getContentPane().add(degreesDMSInput, new org.netbeans.lib.awtextra.AbsoluteConstraints(100, 160, 200, 20));

        momIDLabel1.setText("Seconds:");
        getContentPane().add(momIDLabel1, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 110, -1, 20));

        secondInput.setToolTipText("Seconds in HMS notation");
        getContentPane().add(secondInput, new org.netbeans.lib.awtextra.AbsoluteConstraints(100, 110, 200, -1));

        jLabel2.setText("Degrees:");
        getContentPane().add(jLabel2, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 160, 150, 20));

        convertFromInput.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "HMS", "DMS", "Radians", "Degrees" }));
        convertFromInput.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                convertFromInputActionPerformed(evt);
            }
        });
        getContentPane().add(convertFromInput, new org.netbeans.lib.awtextra.AbsoluteConstraints(100, 360, 90, -1));

        jLabel4.setText("Convert from:");
        getContentPane().add(jLabel4, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 360, -1, -1));

        hourInput.setToolTipText("Degrees in DMS notation");
        getContentPane().add(hourInput, new org.netbeans.lib.awtextra.AbsoluteConstraints(100, 50, 200, 20));

        jLabel5.setText("Minutes:");
        getContentPane().add(jLabel5, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 190, 150, 20));

        minuteDMSInput.setToolTipText("Minutes in HMS notation");
        getContentPane().add(minuteDMSInput, new org.netbeans.lib.awtextra.AbsoluteConstraints(100, 190, 200, -1));

        momIDLabel2.setText("Seconds:");
        getContentPane().add(momIDLabel2, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 220, 150, 20));

        secondDMSInput.setToolTipText("Seconds in HMS notation");
        getContentPane().add(secondDMSInput, new org.netbeans.lib.awtextra.AbsoluteConstraints(100, 220, 200, -1));

        jLabel7.setFont(new java.awt.Font("Tahoma", 1, 11)); // NOI18N
        jLabel7.setText("DMS");
        getContentPane().add(jLabel7, new org.netbeans.lib.awtextra.AbsoluteConstraints(180, 140, 50, -1));

        jLabel8.setFont(new java.awt.Font("Tahoma", 1, 11)); // NOI18N
        jLabel8.setText("HMS");
        getContentPane().add(jLabel8, new org.netbeans.lib.awtextra.AbsoluteConstraints(180, 30, -1, -1));

        pack();
    }// </editor-fold>//GEN-END:initComponents


    
    private void cancelButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_cancelButtonActionPerformed
        setVisible(false);
        dispose();
    }//GEN-LAST:event_cancelButtonActionPerformed

    private void clearFieldsButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_clearFieldsButtonActionPerformed
    degreesDMSInput.setText("");
    minuteInput.setText("");
    secondInput.setText("");
    hourInput.setText("");
    minuteDMSInput.setText("");
    secondDMSInput.setText("");
    radianInput.setText("");
    degreesInput.setText("");
}//GEN-LAST:event_clearFieldsButtonActionPerformed

    private void convertFromInputActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_convertFromInputActionPerformed
        if (evt.getActionCommand().equals("comboBoxChanged")) {
            degreesDMSInput.setBackground(Color.white);
            minuteInput.setBackground(Color.white);
            secondInput.setBackground(Color.white);
            hourInput.setBackground(Color.white);
            minuteDMSInput.setBackground(Color.white);
            secondDMSInput.setBackground(Color.white);
            radianInput.setBackground(Color.white);
            degreesInput.setBackground(Color.white);
            String aS = convertFromInput.getSelectedItem().toString();
            boolean error=false;
            if (aS.contains("DMS")) {
                if (degreesDMSInput.getText().isEmpty()) {
                    degreesDMSInput.setBackground(Color.red);
                    error=true;
                }
                if (minuteDMSInput.getText().isEmpty()) {
                    minuteDMSInput.setBackground(Color.red);
                    error=true;
                }
                if (secondDMSInput.getText().isEmpty()) {
                    secondDMSInput.setBackground(Color.red);
                    error=true;
                }
                if (error) return;
                String dms=degreesDMSInput.getText()+"\u00b0"+minuteDMSInput.getText()+"\'"+secondDMSInput.getText()+"\"";
                try {
                    String hms = LofarUtils.dms2hms(dms);
                    double deg = LofarUtils.dms2deg(dms);
                    double rad = LofarUtils.dms2rad(dms);

                    // split HMS
                    String[] values = hms.split(":");
                    hourInput.setText(values[0]);
                    minuteInput.setText(values[1]);
                    secondInput.setText(values[2]);

                    radianInput.setText(String.valueOf(rad));
                    degreesInput.setText(String.valueOf(deg));

                } catch (NumberFormatException ex) {
                    degreesDMSInput.setBackground(Color.red);
                    minuteDMSInput.setBackground(Color.red);
                    secondDMSInput.setBackground(Color.red);
                } catch (ArrayIndexOutOfBoundsException ex) {
                    System.out.println("Error during split: "+ex);
                    degreesDMSInput.setBackground(Color.red);
                    minuteDMSInput.setBackground(Color.red);
                    secondDMSInput.setBackground(Color.red);
                }
            } else if (aS.contains("HMS")) {
                if (hourInput.getText().isEmpty()) {
                    hourInput.setBackground(Color.red);
                    error=true;
                }
                if (minuteInput.getText().isEmpty()) {
                    minuteInput.setBackground(Color.red);
                    error=true;
                }
                if (secondInput.getText().isEmpty()) {
                    secondInput.setBackground(Color.red);
                    error=true;
                }
                if (error) return;
                String hms=hourInput.getText()+":"+minuteInput.getText()+":"+secondInput.getText();
                try {
                    String dms = LofarUtils.hms2dms(hms);
                    double deg = LofarUtils.hms2deg(hms);
                    double rad = LofarUtils.hms2rad(hms);

                    // split DMS
                    String[] v1 = dms.split("\u00b0");
                    degreesDMSInput.setText(v1[0]);
                    String[] v2 = v1[1].split("\'");
                    minuteDMSInput.setText(v2[0]);
                    String[] v3 = v2[1].split("\"");
                    secondDMSInput.setText(v3[0]);

                    radianInput.setText(String.valueOf(rad));
                    degreesInput.setText(String.valueOf(deg));

                } catch (NumberFormatException ex) {
                    hourInput.setBackground(Color.red);
                    minuteInput.setBackground(Color.red);
                    secondInput.setBackground(Color.red);
                } catch (ArrayIndexOutOfBoundsException ex) {
                    System.out.println("Error during split: "+ex);
                    hourInput.setBackground(Color.red);
                    minuteInput.setBackground(Color.red);
                    secondInput.setBackground(Color.red);
                }
            } else if (aS.contains("Radians")) {
                if (radianInput.getText().isEmpty()) {
                    radianInput.setBackground(Color.red);
                    return;
                }
                try {
                    double rad = Double.valueOf(radianInput.getText());

                    String dms = LofarUtils.rad2dms(rad);
                    String hms = LofarUtils.rad2hms(rad);
                    double deg = LofarUtils.rad2deg(rad);

                    // split DMS
                    String[] v1 = dms.split("\u00b0");
                    degreesDMSInput.setText(v1[0]);
                    String[] v2 = v1[1].split("\'");
                    minuteDMSInput.setText(v2[0]);
                    String[] v3 = v2[1].split("\"");
                    secondDMSInput.setText(v3[0]);

                    // split HMS
                    String[] values = hms.split(":");
                    hourInput.setText(values[0]);
                    minuteInput.setText(values[1]);
                    secondInput.setText(values[2]);

                    degreesInput.setText(String.valueOf(deg));

                } catch (NumberFormatException ex) {
                    radianInput.setBackground(Color.red);
                } catch (ArrayIndexOutOfBoundsException ex) {
                    System.out.println("Error during split: "+ex);
                    radianInput.setBackground(Color.red);
                }
            } else if (aS.contains("Degrees")) {
                if (degreesInput.getText().isEmpty()) {
                    degreesInput.setBackground(Color.red);
                    return;
                }
                try {
                    double deg = Double.valueOf(degreesInput.getText());

                    String dms = LofarUtils.deg2dms(deg);
                    String hms = LofarUtils.deg2hms(deg);
                    double rad = LofarUtils.deg2rad(deg);

                    // split DMS
                    String[] v1 = dms.split("\u00b0");
                    degreesDMSInput.setText(v1[0]);
                    String[] v2 = v1[1].split("\'");
                    minuteDMSInput.setText(v2[0]);
                    String[] v3 = v2[1].split("\"");
                    secondDMSInput.setText(v3[0]);

                    // split HMS
                    String[] values = hms.split(":");
                    hourInput.setText(values[0]);
                    minuteInput.setText(values[1]);
                    secondInput.setText(values[2]);

                    radianInput.setText(String.valueOf(rad));

                } catch (NumberFormatException ex) {
                    degreesInput.setBackground(Color.red);
                } catch (ArrayIndexOutOfBoundsException ex) {
                    System.out.println("Error during split: "+ex);
                    degreesInput.setBackground(Color.red);
                }
            }
        }
}//GEN-LAST:event_convertFromInputActionPerformed


    private MainFrame itsMainFrame = null;
    private boolean   isInitialized=false;
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JButton cancelButton;
    private javax.swing.JButton clearFieldsButton;
    private javax.swing.JComboBox convertFromInput;
    private javax.swing.JTextField degreesDMSInput;
    private javax.swing.JTextField degreesInput;
    private javax.swing.JTextField hourInput;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JLabel jLabel10;
    private javax.swing.JLabel jLabel2;
    private javax.swing.JLabel jLabel3;
    private javax.swing.JLabel jLabel4;
    private javax.swing.JLabel jLabel5;
    private javax.swing.JLabel jLabel6;
    private javax.swing.JLabel jLabel7;
    private javax.swing.JLabel jLabel8;
    private javax.swing.JTextField minuteDMSInput;
    private javax.swing.JTextField minuteInput;
    private javax.swing.JLabel momIDLabel1;
    private javax.swing.JLabel momIDLabel2;
    private javax.swing.JTextField radianInput;
    private javax.swing.JTextField secondDMSInput;
    private javax.swing.JTextField secondInput;
    private javax.swing.JTextArea topLabel;
    // End of variables declaration//GEN-END:variables

}

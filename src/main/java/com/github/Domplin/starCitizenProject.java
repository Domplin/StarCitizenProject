/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 */

package com.github.Domplin;

import java.awt.Rectangle;



/**
 *
 * @author domplin
 */
public class starCitizenProject {

public static void main(String[] args) throws Exception {
    System.setProperty("jna.library.path", "/usr/lib");
    screenAreaFinder sAreaFinder = new screenAreaFinder();
    Rectangle region = sAreaFinder.find();

    screenReader sReader = new screenReader(region, 500);
    sReader.startMonitoring();
}
}

package com.github.Domplin;

import java.awt.*;
import java.awt.image.BufferedImage;
import java.io.File;

import javax.imageio.ImageIO;
import javax.swing.RowFilter;


public class screenAreaFinder {
    
    public static Rectangle find() throws Exception{
        Robot robot = new Robot();
        System.out.println("Move mouse cursor to target area");
        for(int i = 5; i > 0; i--){
            System.out.println("Capturing in " + i + "seconds...");
            Thread.sleep(1000);
        }

        Point p = MouseInfo.getPointerInfo().getLocation();
        System.out.println("Mouse position: X =" + p.x + ", Y =" + p.y);
        
        Rectangle region = new Rectangle(p.x - 100, p.y - 25, 200, 50);
        BufferedImage img = robot.createScreenCapture(region);
        ImageIO.write(img, "png", new File("captured_region.png"));
        System.out.println("Saved to captured_region.png - Check if right area was captured\n");
    
        return region;
    }
}

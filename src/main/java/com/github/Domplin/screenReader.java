package com.github.Domplin;

import net.sourceforge.tess4j.Tesseract;
import  net.sourceforge.tess4j.TesseractException;

import java.awt.*;
import java.awt.image.BufferedImage;
import java.util.ArrayList;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;


public class screenReader {
    
    private final Robot robot;
    private final Tesseract tesseract;
    private final Rectangle captureArea;
    private final int pollIntervalMs;

    public screenReader(Rectangle region, int pollIntervalMs) throws AWTException {
        this.robot = new Robot();
        this.captureArea = region;
        this.pollIntervalMs = pollIntervalMs;


        String tessdata = new java.io.File("tessdata").getAbsolutePath(); //Find the absolute path of tesseract data

        this.tesseract = new Tesseract();
        tesseract.setDatapath(tessdata);
        tesseract.setVariable("tessedit_char_whitelist", "0123456789.");
        tesseract.setPageSegMode(7);
    }


    public String captureText() throws TesseractException {
        BufferedImage screenshot = robot.createScreenCapture(captureArea);
        BufferedImage scaled = scaleImage(screenshot, 3);
        return tesseract.doOCR(scaled).trim();
    }


    public List<Double> extractNumbers(String text){
        List<Double> numbers = new ArrayList<>();
        Pattern pattern = Pattern.compile("-?\\d+(\\.\\d+)?");
        Matcher matcher = pattern.matcher(text);
        while(matcher.find()){
            try{  
                numbers.add(Double.parseDouble(matcher.group()));
            } catch (NumberFormatException e) {
            }

        } 
        return numbers;
    }

    public void startMonitoring(){
        System.out.println("Montoring Screen region: " + captureArea);
        System.out.println("Press Ctrl + C to stop.\n");

        String lastText = "";

        while (true){
            try{
                String text = captureText();

                if(!text.equals(lastText)){
                    lastText = text;
                    List<Double> numbers = extractNumbers(text);
                    System.out.println("Raw OCR: " + text);
                    System.out.println("Numbers: " + numbers);
                    System.out.println("---------------------------------");
                }

                Thread.sleep(pollIntervalMs);
            } catch (TesseractException e) {
                System.err.println("OCR Error: " + e.getMessage());
            } catch (InterruptedException e){
                System.out.println("Monitoring Stopped");
                break;
            }
        }
    }


    private BufferedImage scaleImage(BufferedImage original, int factor){
        int w = original.getWidth() * factor;
        int h = original.getHeight() * factor;
        BufferedImage scaled = new BufferedImage(w, h, BufferedImage.TYPE_INT_RGB);
        Graphics2D g = scaled.createGraphics();
        g.setRenderingHint(RenderingHints.KEY_INTERPOLATION, RenderingHints.VALUE_INTERPOLATION_BICUBIC);
        g.drawImage(original, 0, 0, w, h, null);
        g.dispose();
        return scaled;
    }
    



}

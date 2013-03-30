/**
 * @file POSWriter.java
 * Contains some simple preprocessing code in Java using the Stanford Parser.
 */

import java.io.BufferedReader;
import java.io.FileReader;
import java.util.ArrayList;
import java.util.List;
import java.io.FileWriter;
import java.io.IOException;
import edu.stanford.nlp.ling.Sentence;
import edu.stanford.nlp.ling.TaggedWord;
import edu.stanford.nlp.ling.HasWord;
import edu.stanford.nlp.tagger.maxent.MaxentTagger;

class POSWriter
{
    /**
     * Reads a file and returns all its lines.
     * @param filename - the file to read from
     * @return lines an ArrayList of lines from the file
     */
    private static ArrayList<String> readLines(String filename)
    {
        ArrayList<String> lines = new ArrayList<String>();
        try
        {
            String line = null;
            BufferedReader bufferedReader = new BufferedReader(new FileReader(filename));
            while((line = bufferedReader.readLine()) != null)
                lines.add(line);
            bufferedReader.close();
        }
        catch(Exception e)
        {
            System.out.println("Error reading " + filename);
        }
        return lines;
    }

    public static void main(String[] args) throws Exception
    {
        ArrayList<String> lines = readLines(args[0]);
        System.out.println("Processing " + lines.size() + " files.");
        String modelPath = "../../data/english-left3words-distsim.tagger";
        MaxentTagger tagger = new MaxentTagger(modelPath);
        int filesDone = 0;
        for(String filename: lines)
        {
            FileWriter writer = new FileWriter(filename + ".pos");
            System.out.println("Parsing " + filename + " ... ");
            String numDone = "[" + (filesDone + 1) + "/" + lines.size() + "]: ";
            List<List<HasWord>> sentences = MaxentTagger.tokenizeText(new BufferedReader(new FileReader(filename)));
            double sentencesProcessed = 0;
            for(List<HasWord> sentence: sentences)
            {
                ArrayList<TaggedWord> tSentence = tagger.tagSentence(sentence);
                String tags = "<s> ";
                for(TaggedWord tw: tSentence)
                    tags += tw.tag() + " ";
                tags += "</s>";
                System.out.print("  " + numDone);
                System.out.printf("%2.2f%% ", sentencesProcessed / sentences.size() * 100.0);
                System.out.println(tags.substring(0, Math.min(100, tags.length())));
                ++sentencesProcessed;
                writer.write(tags + "\n");
            }
            System.out.println();
            writer.close();
            ++filesDone;
        }
    }
}
